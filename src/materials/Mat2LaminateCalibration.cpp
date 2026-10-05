#include "alien_evolution/materials/Mat2LaminateCalibration.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace
{
    using Matrix3 = std::array<double, 9>;
    using Matrix6 = std::array<double, 36>;
    using Vector3 = std::array<double, 3>;
    using Vector6 = std::array<double, 6>;

    constexpr std::array<std::size_t, 3> kTangential{0, 1, 5};
    constexpr std::array<std::size_t, 3> kNormal{2, 3, 4};

    template <std::size_t N>
    std::array<double, N * N> invertMatrix(
        const std::array<double, N * N>& input)
    {
        long double augmented[N][2 * N]{};
        long double scale = 0.0L;
        for (std::size_t i = 0; i < N; ++i)
        {
            for (std::size_t j = 0; j < N; ++j)
            {
                augmented[i][j] = input[N * i + j];
                scale = std::max(scale, std::abs(augmented[i][j]));
            }
            augmented[i][N + i] = 1.0L;
        }
        if (!(scale > 0.0L))
            throw std::domain_error("Elastic matrix inversion received a zero matrix.");

        for (std::size_t column = 0; column < N; ++column)
        {
            std::size_t pivot = column;
            long double pivotMagnitude = std::abs(augmented[pivot][column]);
            for (std::size_t row = column + 1; row < N; ++row)
            {
                const auto candidate = std::abs(augmented[row][column]);
                if (candidate > pivotMagnitude)
                {
                    pivot = row;
                    pivotMagnitude = candidate;
                }
            }

            const auto threshold =
                128.0L * std::numeric_limits<long double>::epsilon() * scale;
            if (!(pivotMagnitude > threshold))
                throw std::domain_error(
                    "Elastic matrix inversion is singular or numerically unresolved.");

            if (pivot != column)
                for (std::size_t j = 0; j < 2 * N; ++j)
                    std::swap(augmented[pivot][j], augmented[column][j]);

            const auto pivotValue = augmented[column][column];
            for (std::size_t j = 0; j < 2 * N; ++j)
                augmented[column][j] /= pivotValue;

            for (std::size_t row = 0; row < N; ++row)
            {
                if (row == column) continue;
                const auto factor = augmented[row][column];
                for (std::size_t j = 0; j < 2 * N; ++j)
                    augmented[row][j] -= factor * augmented[column][j];
            }
        }

        std::array<double, N * N> result{};
        for (std::size_t i = 0; i < N; ++i)
            for (std::size_t j = 0; j < N; ++j)
            {
                const auto value = augmented[i][N + j];
                if (!std::isfinite(static_cast<double>(value)))
                    throw std::overflow_error(
                        "Elastic matrix inverse is not representable.");
                result[N * i + j] = static_cast<double>(value);
            }
        return result;
    }

    Matrix3 extractBlock(
        const Matrix6& matrix,
        const std::array<std::size_t, 3>& rows,
        const std::array<std::size_t, 3>& columns)
    {
        Matrix3 result{};
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                result[3 * i + j] = matrix[6 * rows[i] + columns[j]];
        return result;
    }

    Matrix3 add3(const Matrix3& a, const Matrix3& b)
    {
        Matrix3 result{};
        for (std::size_t i = 0; i < result.size(); ++i)
            result[i] = a[i] + b[i];
        return result;
    }

    Matrix3 subtract3(const Matrix3& a, const Matrix3& b)
    {
        Matrix3 result{};
        for (std::size_t i = 0; i < result.size(); ++i)
            result[i] = a[i] - b[i];
        return result;
    }

    Matrix3 scale3(const Matrix3& a, double scalar)
    {
        Matrix3 result{};
        for (std::size_t i = 0; i < result.size(); ++i)
            result[i] = scalar * a[i];
        return result;
    }

    Matrix3 multiply3(const Matrix3& a, const Matrix3& b)
    {
        Matrix3 result{};
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                for (std::size_t k = 0; k < 3; ++k)
                    result[3 * i + j] += a[3 * i + k] * b[3 * k + j];
        return result;
    }

    Vector3 multiply3(const Matrix3& a, const Vector3& x)
    {
        Vector3 result{};
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                result[i] += a[3 * i + j] * x[j];
        return result;
    }

    Matrix3 transpose3(const Matrix3& a)
    {
        return {
            a[0], a[3], a[6],
            a[1], a[4], a[7],
            a[2], a[5], a[8]};
    }

    Vector3 add3(const Vector3& a, const Vector3& b)
    {
        return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
    }

    Vector3 subtract3(const Vector3& a, const Vector3& b)
    {
        return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
    }

    Matrix6 weightedMatrix6(
        const Matrix6& a,
        const Matrix6& b,
        double fA)
    {
        Matrix6 result{};
        const auto fB = 1.0 - fA;
        for (std::size_t i = 0; i < result.size(); ++i)
            result[i] = fA * a[i] + fB * b[i];
        return result;
    }

    struct LaminateBlocks
    {
        Matrix3 invNnA;
        Matrix3 invNnB;
        Matrix3 ntA;
        Matrix3 ntB;
        Matrix3 A;
        Matrix3 B;
        Matrix3 invA;
    };

    LaminateBlocks laminateBlocks(
        const ae::KelvinElasticityTensor& phaseA,
        const ae::KelvinElasticityTensor& phaseB,
        double fA)
    {
        const auto fB = 1.0 - fA;
        const auto& ca = phaseA.matrix();
        const auto& cb = phaseB.matrix();

        const auto nnA = extractBlock(ca, kNormal, kNormal);
        const auto nnB = extractBlock(cb, kNormal, kNormal);
        const auto ntA = extractBlock(ca, kNormal, kTangential);
        const auto ntB = extractBlock(cb, kNormal, kTangential);
        const auto invNnA = invertMatrix<3>(nnA);
        const auto invNnB = invertMatrix<3>(nnB);

        const auto A = add3(scale3(invNnA, fA), scale3(invNnB, fB));
        const auto B = add3(
            scale3(multiply3(invNnA, ntA), fA),
            scale3(multiply3(invNnB, ntB), fB));

        return {
            invNnA,
            invNnB,
            ntA,
            ntB,
            A,
            B,
            invertMatrix<3>(A)};
    }

    ae::KelvinElasticityTensor exactLocalLaminateTensor(
        const ae::KelvinElasticityTensor& phaseA,
        const ae::KelvinElasticityTensor& phaseB,
        double fA)
    {
        const auto fB = 1.0 - fA;
        const auto& ca = phaseA.matrix();
        const auto& cb = phaseB.matrix();

        const auto blocks = laminateBlocks(phaseA, phaseB, fA);

        const auto ttA = extractBlock(ca, kTangential, kTangential);
        const auto ttB = extractBlock(cb, kTangential, kTangential);
        const auto tnA = extractBlock(ca, kTangential, kNormal);
        const auto tnB = extractBlock(cb, kTangential, kNormal);

        const auto correctionA =
            multiply3(multiply3(tnA, blocks.invNnA), blocks.ntA);
        const auto correctionB =
            multiply3(multiply3(tnB, blocks.invNnB), blocks.ntB);

        const auto D = add3(
            scale3(subtract3(ttA, correctionA), fA),
            scale3(subtract3(ttB, correctionB), fB));

        const auto effNn = blocks.invA;
        const auto effNt = multiply3(blocks.invA, blocks.B);
        const auto effTn = transpose3(effNt);
        const auto effTt = add3(
            D,
            multiply3(
                multiply3(transpose3(blocks.B), blocks.invA),
                blocks.B));

        Matrix6 effective{};
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
            {
                effective[6 * kTangential[i] + kTangential[j]] = effTt[3 * i + j];
                effective[6 * kTangential[i] + kNormal[j]] = effTn[3 * i + j];
                effective[6 * kNormal[i] + kTangential[j]] = effNt[3 * i + j];
                effective[6 * kNormal[i] + kNormal[j]] = effNn[3 * i + j];
            }

        for (std::size_t i = 0; i < 6; ++i)
            for (std::size_t j = i + 1; j < 6; ++j)
            {
                const auto value =
                    0.5 * (effective[6 * i + j] + effective[6 * j + i]);
                effective[6 * i + j] = value;
                effective[6 * j + i] = value;
            }

        return ae::KelvinElasticityTensor(effective);
    }

    ae::KelvinElasticityTensor reussLowerBound(
        const ae::KelvinElasticityTensor& phaseA,
        const ae::KelvinElasticityTensor& phaseB,
        double fA)
    {
        const auto invA = invertMatrix<6>(phaseA.matrix());
        const auto invB = invertMatrix<6>(phaseB.matrix());
        const auto averageCompliance = weightedMatrix6(invA, invB, fA);
        return ae::KelvinElasticityTensor(invertMatrix<6>(averageCompliance));
    }

    ae::PhysicalQuantityDescriptor elasticityQuantity()
    {
        return {
            "elasticity_tensor",
            "mat2.effective_kelvin_elasticity_tensor",
            "Pa",
            "Exact MAT-2 periodic-laminate energy-based 3-D elasticity operator"};
    }

    ae::AdaptivePhysicsContract makeContract()
    {
        ae::AdaptivePhysicsContract contract{
            ae::ScientificModelRef("alien_evolution.mat2.elasticity_operator", "mat2-v1"),
            {elasticityQuantity()},
            {
                {"kinematics", "mat2.infinitesimal_strain",
                    "Geometrically linear infinitesimal-strain calibration only.",
                    "Finite deformation/large rotation requires a later mechanics model."},
                {"time_regime", "mat2.quasi_static",
                    "Quasi-static constitutive/homogenization reference; inertia and waves are absent.",
                    "Do not extrapolate to dynamic/nonlocal effective mechanics."},
                {"thermal_regime", "mat2.isothermal",
                    "No thermoelastic coupling or temperature evolution.",
                    std::nullopt},
                {"constitutive_family", "mat2.energy_linear_elasticity",
                    "Linear, energy-based, major-symmetric positive-definite elasticity.",
                    "Plasticity, viscosity, damage, fracture and active stress are excluded."},
                {"microstructure", "mat2.perfect_periodic_laminate",
                    "Perfectly bonded periodic two-phase laminate in the homogenized limit.",
                    "Finite-scale RVE adequacy is not certified."}
            },
            {
                {"closed_form_reference_difference", "elasticity_tensor",
                    "mat2.exact_layered_reference_difference", "Pa",
                    "Numerical difference from exact analytical laminate block condensation."},
                {"hill_mandel_residual", "elasticity_tensor",
                    "mat2.hill_mandel_work_residual", "Pa",
                    "Micro/macro work-density residual for analytical local fields."}
            },
            {},
            "MAT-2 constitutive/homogenization reference only. It does not solve organism structural equilibrium."};
        contract.validate();
        return contract;
    }

    ae::ScientificModelMetadata makeMetadata()
    {
        ae::ScientificModelMetadata metadata;
        metadata.identifier = "alien_evolution.mat2.elasticity_operator";
        metadata.name = "MAT-2 exact periodic-laminate 3-D elasticity";
        metadata.version = "mat2-v1";
        metadata.description =
            "Exact quasi-static small-strain energy-based 3-D constitutive/homogenization calibration for a two-phase periodic laminate. "
            "No organism mechanics PDE or numerical RVE solver is included.";
        metadata.parameters = {
            {"K_A", "Pa", "Benchmark phase-A bulk modulus"},
            {"G_A", "Pa", "Benchmark phase-A shear modulus"},
            {"K_B", "Pa", "Benchmark phase-B bulk modulus"},
            {"G_B", "Pa", "Benchmark phase-B shear modulus"},
            {"f_A", "dimensionless", "Phase-A volume fraction"},
            {"period", "m", "Periodic laminate characteristic scale"}};
        metadata.assumptions = {
            "Isothermal quasi-static response",
            "Infinitesimal strain / geometrically linear kinematics",
            "Energy-based linear elasticity with positive benchmark K and G",
            "Two abstract homogeneous isotropic benchmark phases",
            "Perfectly bonded periodic planar laminate",
            "Exact analytical homogenized limit with no finite-scale RVE certification",
            "No inertia, waves, nonlocality, plasticity, viscosity, damage, fracture or active stress",
            "Benchmark constituent K/G are simulator/reference inputs, not mature heritable traits"};
        metadata.validityScope =
            "MAT-2 exact periodic-laminate constitutive/homogenization calibration only. "
            "Structural equilibrium, dynamic homogenization, finite strain, failure and general microstructures remain unresolved.";
        metadata.uncertainties = {
            ae::UncertaintyKind::NumericalReduction,
            ae::UncertaintyKind::ScientificModelForm};
        metadata.evidence = {{
            ae::EvidenceStatus::EstablishedPhysicalInteraction,
            "Periodic layered composites admit exact effective properties and local fields; tensor-level Voigt/Reuss interpretations and micro/macro energetic consistency provide strong analytical checks.",
            {
                {"Pindera et al. (2012)", "https://doi.org/10.1016/j.mechrescom.2012.08.007",
                    "Exact periodic layered-composite effective properties and local fields"},
                {"Ostoja-Starzewski (2006)", "https://doi.org/10.1016/j.probengmech.2005.07.007",
                    "Scale-dependent SVE/RVE concepts and bounds"},
                {"Nagel et al. (2016)", "https://doi.org/10.1007/s12665-016-5429-4",
                    "Kelvin mapping advantages for tensor-consistent numerical implementation"}
            },
            "MAT-2 uses a static analytical laminate reference and does not validate general organism mechanics."}};
        metadata.validate();
        return metadata;
    }

    std::vector<ae::MaterialScienceUpdateBinding> updateBindings()
    {
        return {
            {std::string(ae::material_update_points::ConstituentPhysics),
                ae::ScientificModelRef("alien_evolution.mat2.isotropic_KG_constituents", "mat2-v1"),
                "Positive bulk/shear benchmark constituent response; replaceable without changing heredity."},
            {std::string(ae::material_update_points::Homogenization),
                ae::ScientificModelRef("alien_evolution.mat2.exact_periodic_laminate", "mat2-v1"),
                "Exact analytical layer block condensation; no numerical RVE solver."},
            {std::string(ae::material_update_points::ConstitutiveResponse),
                ae::ScientificModelRef("alien_evolution.mat2.energy_linear_elasticity", "mat2-v1"),
                "Small-strain energy-based 3-D elasticity."},
            {std::string(ae::material_update_points::NumericalRealization),
                ae::ScientificModelRef("alien_evolution.mat2.closed_form_kelvin", "mat2-v1"),
                "Closed-form block algebra in Mandel-Kelvin representation."},
            {std::string(ae::material_update_points::CalibrationReference),
                ae::ScientificModelRef("alien_evolution.mat2.layered_composite_reference", "mat2-v1"),
                "Pindera-style exact periodic layered-composite calibration."}
        };
    }

    ae::MaterialResponseQueryDescriptor queryDescriptor(
        const ae::Mat2ElasticResponseQuery& query)
    {
        return {
            "mat2_effective_elasticity",
            "mechanical.linear_elastic_constitutive_response",
            "elasticity_tensor",
            query.requestedSpatialScale,
            std::nullopt,
            "mat2.periodic_homogenized_quasistatic_small_strain",
            "Compile exact periodic-laminate 3-D elasticity tensor"};
    }

    ae::MaterialCompilationRecord compiledRecord(
        const ae::Mat2LaminateMaterialState& state,
        const ae::Mat2ElasticResponseQuery& query)
    {
        const ae::ScientificModelRef model(
            "alien_evolution.mat2.elasticity_operator",
            "mat2-v1");

        ae::MaterialCompilationRecord record{
            state.envelope.identifier(),
            queryDescriptor(query),
            ae::MaterialCompilationDisposition::Compiled,
            model,
            makeContract(),
            updateBindings(),
            {ae::DerivedArtifactId(state.envelope.identifier())},
            {},
            {
                {"closed_form_block_condensation", 1.0, "evaluation",
                    "Simulator cost metadata only; no mesh/RVE solve is performed."}
            },
            "Exact periodic homogenized-limit constitutive response; finite-scale RVE adequacy remains uncertified."};
        record.validate();
        return record;
    }

    ae::KelvinElasticityTensor voigtUpperBound(
        const ae::KelvinElasticityTensor& phaseA,
        const ae::KelvinElasticityTensor& phaseB,
        double fA)
    {
        return ae::KelvinElasticityTensor(
            weightedMatrix6(phaseA.matrix(), phaseB.matrix(), fA));
    }

    Vector3 subset(const Vector6& value, const std::array<std::size_t, 3>& indices)
    {
        return {value[indices[0]], value[indices[1]], value[indices[2]]};
    }

    Vector6 assemble(
        const Vector3& tangential,
        const Vector3& normal)
    {
        Vector6 value{};
        for (std::size_t i = 0; i < 3; ++i)
        {
            value[kTangential[i]] = tangential[i];
            value[kNormal[i]] = normal[i];
        }
        return value;
    }
}

namespace ae
{
    void Mat2BenchmarkPhase::validate() const
    {
        if (identifier.empty())
            throw std::invalid_argument("MAT-2 phase identifier must not be empty.");
        elasticity.validate();
    }

    Mat2LaminateMaterialState::Mat2LaminateMaterialState(
        std::string stateIdentifier,
        Mat2BenchmarkPhase phaseAInput,
        Mat2BenchmarkPhase phaseBInput,
        double phaseAFractionInput,
        double laminatePeriodInput,
        std::array<double, 3> layerNormalInput)
        : envelope(
            stateIdentifier,
            ScientificModelRef("alien_evolution.mat2.laminate_material_state", "mat2-v1"),
            laminatePeriodInput,
            laminatePeriodInput,
            {
                {"composition",
                    ScientificModelRef("alien_evolution.mat2.two_phase_volume_fraction", "mat2-v1"),
                    laminatePeriodInput,
                    "Two abstract isotropic phase fractions; K/G payload is MAT-2 benchmark-specific."},
                {"microstructure",
                    ScientificModelRef("alien_evolution.mat2.periodic_planar_laminate", "mat2-v1"),
                    laminatePeriodInput,
                    "Perfectly bonded periodic planar layers."},
                {"orientation",
                    ScientificModelRef("alien_evolution.mat2.layer_normal", "mat2-v1"),
                    laminatePeriodInput,
                    "Layer-normal orientation; effective anisotropy derives from this structure."}
            }),
          phaseA(std::move(phaseAInput)),
          phaseB(std::move(phaseBInput)),
          phaseAFraction(phaseAFractionInput),
          laminatePeriod(laminatePeriodInput),
          layerNormal({
              Rotation3::fromLayerNormal(layerNormalInput).at(0, 2),
              Rotation3::fromLayerNormal(layerNormalInput).at(1, 2),
              Rotation3::fromLayerNormal(layerNormalInput).at(2, 2)})
    {
        validate();
    }

    double Mat2LaminateMaterialState::phaseBFraction() const
    {
        return 1.0 - phaseAFraction;
    }

    void Mat2LaminateMaterialState::validate() const
    {
        envelope.validate();
        phaseA.validate();
        phaseB.validate();

        if (!std::isfinite(phaseAFraction)
            || phaseAFraction < 0.0
            || phaseAFraction > 1.0)
            throw std::invalid_argument("MAT-2 phase-A fraction must lie in [0,1].");

        if (!std::isfinite(laminatePeriod) || laminatePeriod <= 0.0)
            throw std::invalid_argument("MAT-2 laminate period must be finite and positive.");

        if (std::abs(envelope.coarseGrainingLength() - laminatePeriod)
                > 1.0e-12 * std::max(1.0, laminatePeriod)
            || !envelope.characteristicMicrostructureLength()
            || std::abs(*envelope.characteristicMicrostructureLength() - laminatePeriod)
                > 1.0e-12 * std::max(1.0, laminatePeriod))
            throw std::invalid_argument(
                "MAT-2 laminate period must remain consistent with material-state envelope scales.");

        (void)Rotation3::fromLayerNormal(layerNormal);
    }

    void Mat2ElasticResponseQuery::validate() const
    {
        if (requestedSpatialScale
            && (!std::isfinite(*requestedSpatialScale)
                || *requestedSpatialScale <= 0.0))
            throw std::invalid_argument(
                "MAT-2 requested spatial scale must be finite and positive when present.");
    }

    void Mat2ElasticCompilationResult::validate() const
    {
        record.validate();
        effectiveTensor.validate();
        voigtUpperBound.validate();
        reussLowerBound.validate();

        if (record.disposition != MaterialCompilationDisposition::Compiled)
            throw std::invalid_argument("MAT-2 calibration result must be a compiled response.");

        if (requestedScaleToPeriodRatio
            && (!std::isfinite(*requestedScaleToPeriodRatio)
                || *requestedScaleToPeriodRatio <= 0.0))
            throw std::invalid_argument(
                "MAT-2 requested-scale/period ratio must be finite and positive when present.");
    }

    void Mat2PhaseField::validate() const
    {
        (void)strain.kelvin();
        (void)stress.kelvin();
        if (!std::isfinite(elasticEnergyDensity) || elasticEnergyDensity < 0.0)
            throw std::invalid_argument(
                "MAT-2 phase elastic energy density must be finite and nonnegative.");
    }

    void Mat2LaminateMicroscaleResponse::validate(double phaseAFraction) const
    {
        if (!std::isfinite(phaseAFraction)
            || phaseAFraction < 0.0
            || phaseAFraction > 1.0)
            throw std::invalid_argument("MAT-2 response phase fraction is invalid.");
        (void)macroStrain.kelvin();
        (void)macroStress.kelvin();
        phaseA.validate();
        phaseB.validate();
        (void)volumeAverageMicroStrain.kelvin();
        (void)volumeAverageMicroStress.kelvin();

        for (const auto value : {
            macroElasticEnergyDensity,
            volumeAverageMicroElasticEnergyDensity,
            hillMandelWorkResidual,
            energyResidual})
            if (!std::isfinite(value))
                throw std::invalid_argument("MAT-2 microscale response contains nonfinite scalar.");
    }

    Mat2ElasticCompilationResult Mat2LaminateCalibrationCompiler::compile(
        const Mat2LaminateMaterialState& state,
        const Mat2ElasticResponseQuery& query)
    {
        state.validate();
        query.validate();

        const auto phaseATensor = state.phaseA.elasticity.tensor();
        const auto phaseBTensor = state.phaseB.elasticity.tensor();
        const auto localEffective =
            exactLocalLaminateTensor(phaseATensor, phaseBTensor, state.phaseAFraction);

        const auto rotation = Rotation3::fromLayerNormal(state.layerNormal);
        const auto effective = localEffective.rotated(rotation);
        const auto voigt =
            voigtUpperBound(phaseATensor, phaseBTensor, state.phaseAFraction)
                .rotated(rotation);
        const auto reuss =
            reussLowerBound(phaseATensor, phaseBTensor, state.phaseAFraction)
                .rotated(rotation);

        std::optional<double> scaleRatio;
        if (query.requestedSpatialScale)
            scaleRatio = *query.requestedSpatialScale / state.laminatePeriod;

        Mat2ElasticCompilationResult result{
            compiledRecord(state, query),
            effective,
            voigt,
            reuss,
            scaleRatio};
        result.validate();
        return result;
    }

    Mat2LaminateMicroscaleResponse Mat2LaminateCalibrationCompiler::evaluateMicroscale(
        const Mat2LaminateMaterialState& state,
        const SymmetricTensor3& macroStrain)
    {
        state.validate();
        const auto compiled = compile(state);
        const auto phaseATensor = state.phaseA.elasticity.tensor();
        const auto phaseBTensor = state.phaseB.elasticity.tensor();

        const auto localToGlobal = Rotation3::fromLayerNormal(state.layerNormal);
        const auto globalToLocal = localToGlobal.transposed();
        const auto macroLocal = macroStrain.rotated(globalToLocal);
        const auto macroKelvin = macroLocal.kelvin();
        const auto eT = subset(macroKelvin, kTangential);
        const auto eN = subset(macroKelvin, kNormal);

        const auto blocks =
            laminateBlocks(phaseATensor, phaseBTensor, state.phaseAFraction);

        const auto commonNormalStress =
            multiply3(blocks.invA, add3(eN, multiply3(blocks.B, eT)));

        const auto eNA = multiply3(
            blocks.invNnA,
            subtract3(commonNormalStress, multiply3(blocks.ntA, eT)));
        const auto eNB = multiply3(
            blocks.invNnB,
            subtract3(commonNormalStress, multiply3(blocks.ntB, eT)));

        const auto strainALocal = SymmetricTensor3::fromKelvin(assemble(eT, eNA));
        const auto strainBLocal = SymmetricTensor3::fromKelvin(assemble(eT, eNB));
        const auto stressALocal = phaseATensor.stress(strainALocal);
        const auto stressBLocal = phaseBTensor.stress(strainBLocal);

        const auto strainA = strainALocal.rotated(localToGlobal);
        const auto strainB = strainBLocal.rotated(localToGlobal);
        const auto stressA = stressALocal.rotated(localToGlobal);
        const auto stressB = stressBLocal.rotated(localToGlobal);

        const auto fA = state.phaseAFraction;
        const auto fB = state.phaseBFraction();
        const auto averageStrain = strainA * fA + strainB * fB;
        const auto averageStress = stressA * fA + stressB * fB;

        const auto macroStress = compiled.effectiveTensor.stress(macroStrain);
        const auto energyA = phaseATensor.energyDensity(strainALocal);
        const auto energyB = phaseBTensor.energyDensity(strainBLocal);
        const auto averageEnergy = fA * energyA + fB * energyB;
        const auto macroEnergy = compiled.effectiveTensor.energyDensity(macroStrain);
        const auto averageWork =
            fA * stressA.inner(strainA)
            + fB * stressB.inner(strainB);
        const auto macroWork = macroStress.inner(macroStrain);

        Mat2LaminateMicroscaleResponse response{
            macroStrain,
            macroStress,
            {strainA, stressA, energyA},
            {strainB, stressB, energyB},
            averageStrain,
            averageStress,
            macroEnergy,
            averageEnergy,
            averageWork - macroWork,
            averageEnergy - macroEnergy};
        response.validate(fA);
        return response;
    }

    const ScientificModelMetadata& Mat2LaminateCalibrationCompiler::scientificMetadata()
    {
        static const auto metadata = makeMetadata();
        return metadata;
    }
} // namespace ae
