#include "alien_evolution/materials/Mat1LaminateCalibration.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <utility>

namespace
{
    constexpr double kDirectionTolerance = 1.0e-12;

    std::array<double, 3> normalized(std::array<double, 3> value)
    {
        for (const auto component : value)
            if (!std::isfinite(component))
                throw std::invalid_argument("MAT-1 direction components must be finite.");

        const auto norm = std::hypot(value[0], value[1], value[2]);
        if (!std::isfinite(norm) || norm <= 0.0)
            throw std::invalid_argument("MAT-1 direction vector must have finite positive norm.");
        for (auto& component : value) component /= norm;
        return value;
    }

    double absoluteDot(
        const std::array<double, 3>& left,
        const std::array<double, 3>& right)
    {
        double value = 0.0;
        for (std::size_t i = 0; i < left.size(); ++i)
            value += left[i] * right[i];
        return std::abs(value);
    }

    ae::PhysicalQuantityDescriptor axialModulusQuantity()
    {
        return {
            "effective_axial_modulus",
            "mat1.effective_axial_modulus",
            "Pa",
            "Scalar effective modulus for the exact MAT-1 one-dimensional calibration only"};
    }

    ae::AdaptivePhysicsContract makeContract(ae::Mat1AxialCalibrationMode mode)
    {
        const auto regime = mode == ae::Mat1AxialCalibrationMode::InPlaneIsoStrain
            ? "mat1.periodic_1d_iso_strain_parallel_layers"
            : "mat1.periodic_1d_iso_stress_normal_layers";

        ae::AdaptivePhysicsContract contract{
            ae::ScientificModelRef("alien_evolution.mat1.axial_compiled_response", "mat1-v1"),
            {axialModulusQuantity()},
            {
                {"one_dimensional_calibration", regime,
                    "Exact only for the declared scalar one-dimensional laminate boundary-value problem.",
                    "Do not reinterpret as a full 3-D laminate Young's modulus."},
                {"periodic_homogenized_limit", "mat1.periodic_homogenized_limit",
                    "MAT-1 assumes the periodic homogenized calibration limit.",
                    "Finite-scale RVE adequacy is intentionally not certified by M8A."},
                {"perfect_interface", "mat1.perfect_bond",
                    "The benchmark assumes perfectly bonded phases with no interfacial compliance or damage.",
                    std::nullopt}
            },
            {
                {"calibration_reference_difference", "effective_axial_modulus",
                    "mat1.closed_form_reference_difference", "Pa",
                    "Numerical difference from the exact arithmetic/harmonic one-dimensional reference."}
            },
            {},
            "MAT-1 benchmark ACP declaration. Reference, runtime and compiled material roles remain distinct."};
        contract.validate();
        return contract;
    }

    ae::ScientificModelMetadata makeMetadata()
    {
        ae::ScientificModelMetadata metadata;
        metadata.identifier = "alien_evolution.mat1.axial_compiled_response";
        metadata.name = "MAT-1 one-dimensional laminate calibration response";
        metadata.version = "mat1-v1";
        metadata.description =
            "Exact scalar periodic laminate calibration used to prove material-state + structure + query -> derived response. "
            "Not a general three-dimensional elasticity law.";
        metadata.parameters = {
            {"E_A", "Pa", "Benchmark phase-A one-dimensional axial elastic modulus"},
            {"E_B", "Pa", "Benchmark phase-B one-dimensional axial elastic modulus"},
            {"f_A", "dimensionless", "Phase-A volume fraction; phase B is 1-f_A"},
            {"period", "m", "Laminate microstructure period"}};
        metadata.assumptions = {
            "Two abstract homogeneous benchmark phases",
            "Positive linear scalar one-dimensional constituent moduli",
            "Perfectly bonded periodic laminate",
            "Small one-dimensional axial response in the stated calibration boundary-value problem",
            "Iso-strain arithmetic response only for loading in the layer plane",
            "Iso-stress harmonic response only for loading normal to the layers",
            "No oblique interpolation",
            "No finite-scale RVE certification",
            "Benchmark constituent moduli are simulator/reference inputs, not heritable material traits"};
        metadata.validityScope =
            "MAT-1 scalar one-dimensional periodic laminate calibration only; full tensor elasticity, "
            "Poisson coupling, finite-scale representativity, damage and nonlinear mechanics remain unresolved.";
        metadata.uncertainties = {
            ae::UncertaintyKind::NumericalReduction,
            ae::UncertaintyKind::ScientificModelForm};
        metadata.evidence = {{
            ae::EvidenceStatus::EstablishedPhysicalInteraction,
            "Layered composites have structure- and orientation-dependent effective response; exact full layered-composite solutions require tensor-aware homogenization beyond MAT-1.",
            {
                {"Pindera et al. (2012)", "https://doi.org/10.1016/j.mechrescom.2012.08.007",
                    "Exact effective properties/local fields for periodic layered composites"},
                {"Ostoja-Starzewski (2006)", "https://doi.org/10.1016/j.probengmech.2005.07.007",
                    "Scale-dependent SVE/RVE homogenization and bounds"}
            },
            "MAT-1 uses only the exact scalar iso-strain/iso-stress special cases and does not claim full 3-D laminate elasticity."}};
        metadata.validate();
        return metadata;
    }

    std::vector<ae::MaterialScienceUpdateBinding> updateBindings()
    {
        return {
            {std::string(material_update_points::ConstituentPhysics),
                ae::ScientificModelRef("alien_evolution.mat1.benchmark_constituent_axial_elasticity", "mat1-v1"),
                "Reference constituent response inputs; replaceable without changing heredity."},
            {std::string(material_update_points::Homogenization),
                ae::ScientificModelRef("alien_evolution.mat1.periodic_1d_laminate_homogenization", "mat1-v1"),
                "Exact scalar iso-strain / iso-stress laminate calibration."},
            {std::string(material_update_points::ConstitutiveResponse),
                ae::ScientificModelRef("alien_evolution.mat1.linear_axial_response", "mat1-v1"),
                "One-dimensional linear elastic calibration only."},
            {std::string(material_update_points::NumericalRealization),
                ae::ScientificModelRef("alien_evolution.mat1.closed_form_evaluator", "mat1-v1"),
                "Closed-form arithmetic/harmonic evaluation; no mechanics solver."},
            {std::string(material_update_points::CalibrationReference),
                ae::ScientificModelRef("alien_evolution.mat1.layered_composite_reference", "mat1-v1"),
                "Literature-backed layered-composite calibration and exact scalar derivation."}
        };
    }

    ae::MaterialResponseQueryDescriptor queryDescriptor(
        const ae::Mat1AxialResponseQuery& query)
    {
        return {
            "mat1_axial_response",
            "mechanical.axial_linear_response",
            "effective_axial_modulus",
            query.requestedSpatialScale,
            std::nullopt,
            "mat1.periodic_homogenized_1d",
            "MAT-1 exact scalar laminate response query"};
    }

    ae::MaterialCompilationRecord unresolvedRecord(
        const ae::Mat1LaminateMaterialState& state,
        const ae::Mat1AxialResponseQuery& query)
    {
        ae::MaterialCompilationRecord record{
            state.envelope.identifier(),
            queryDescriptor(query),
            ae::MaterialCompilationDisposition::UnsupportedQuery,
            std::nullopt,
            std::nullopt,
            updateBindings(),
            {ae::DerivedArtifactId(state.envelope.identifier())},
            {},
            {},
            "MAT-1 does not interpolate oblique loading; full tensor laminate mechanics is a later scientific model."};
        record.validate();
        return record;
    }

    ae::MaterialCompilationRecord compiledRecord(
        const ae::Mat1LaminateMaterialState& state,
        const ae::Mat1AxialResponseQuery& query,
        ae::Mat1AxialCalibrationMode mode)
    {
        const ae::ScientificModelRef model(
            "alien_evolution.mat1.axial_compiled_response",
            "mat1-v1");
        ae::MaterialCompilationRecord record{
            state.envelope.identifier(),
            queryDescriptor(query),
            ae::MaterialCompilationDisposition::Compiled,
            model,
            makeContract(mode),
            updateBindings(),
            {ae::DerivedArtifactId(state.envelope.identifier())},
            {},
            {
                {"closed_form_operations", 1.0, "evaluation",
                    "Simulator cost metadata only; not material state or an evolvable trait."}
            },
            "Finite-scale representativity is not certified; MAT-1 is the periodic homogenized-limit calibration."};
        record.validate();
        return record;
    }
}

namespace ae
{
    void Mat1BenchmarkPhase::validate() const
    {
        if (identifier.empty())
            throw std::invalid_argument("MAT-1 phase identifier must not be empty.");
        if (!std::isfinite(axialElasticModulus) || axialElasticModulus <= 0.0)
            throw std::invalid_argument(
                "MAT-1 benchmark phase axial modulus must be finite and positive.");
    }

    Mat1LaminateMaterialState::Mat1LaminateMaterialState(
        std::string stateIdentifier,
        Mat1BenchmarkPhase phaseAInput,
        Mat1BenchmarkPhase phaseBInput,
        double phaseAFractionInput,
        double laminatePeriodInput,
        std::array<double, 3> layerNormalInput)
        : envelope(
            stateIdentifier,
            ScientificModelRef("alien_evolution.mat1.laminate_material_state", "mat1-v1"),
            laminatePeriodInput,
            laminatePeriodInput,
            {
                {"composition",
                    ScientificModelRef("alien_evolution.mat1.two_phase_volume_fraction", "mat1-v1"),
                    laminatePeriodInput,
                    "Two abstract phase fractions; benchmark-specific numeric payload lives in MAT-1 state."},
                {"microstructure",
                    ScientificModelRef("alien_evolution.mat1.periodic_laminate", "mat1-v1"),
                    laminatePeriodInput,
                    "Periodic layered topology with perfect interfaces."},
                {"orientation",
                    ScientificModelRef("alien_evolution.mat1.layer_normal", "mat1-v1"),
                    laminatePeriodInput,
                    "Unit layer-normal orientation; anisotropy derives from structure."}
            }),
          phaseA(std::move(phaseAInput)),
          phaseB(std::move(phaseBInput)),
          phaseAFraction(phaseAFractionInput),
          laminatePeriod(laminatePeriodInput),
          layerNormal(normalized(layerNormalInput))
    {
        validate();
    }

    double Mat1LaminateMaterialState::phaseBFraction() const
    {
        return 1.0 - phaseAFraction;
    }

    void Mat1LaminateMaterialState::validate() const
    {
        envelope.validate();
        phaseA.validate();
        phaseB.validate();

        if (!std::isfinite(phaseAFraction)
            || phaseAFraction < 0.0
            || phaseAFraction > 1.0)
            throw std::invalid_argument("MAT-1 phase-A fraction must lie in [0,1].");

        if (!std::isfinite(laminatePeriod) || laminatePeriod <= 0.0)
            throw std::invalid_argument("MAT-1 laminate period must be finite and positive.");

        if (std::abs(envelope.coarseGrainingLength() - laminatePeriod)
                > 1.0e-12 * std::max(1.0, laminatePeriod)
            || !envelope.characteristicMicrostructureLength()
            || std::abs(*envelope.characteristicMicrostructureLength() - laminatePeriod)
                > 1.0e-12 * std::max(1.0, laminatePeriod))
            throw std::invalid_argument(
                "MAT-1 laminate period must remain consistent with the developed material-state envelope scales.");

        const auto normalizedNormal = normalized(layerNormal);
        for (std::size_t i = 0; i < layerNormal.size(); ++i)
            if (std::abs(normalizedNormal[i] - layerNormal[i]) > 1.0e-12)
                throw std::invalid_argument("MAT-1 layer normal must be normalized.");
    }

    Mat1AxialResponseQuery::Mat1AxialResponseQuery(
        std::array<double, 3> loadingDirectionInput,
        std::optional<double> requestedSpatialScaleInput)
        : loadingDirection(normalized(loadingDirectionInput)),
          requestedSpatialScale(requestedSpatialScaleInput)
    {
        validate();
    }

    void Mat1AxialResponseQuery::validate() const
    {
        (void)normalized(loadingDirection);
        if (requestedSpatialScale
            && (!std::isfinite(*requestedSpatialScale)
                || *requestedSpatialScale <= 0.0))
            throw std::invalid_argument(
                "MAT-1 requested spatial scale must be finite and positive when present.");
    }

    void Mat1AxialCompilationResult::validate() const
    {
        record.validate();

        if (record.disposition == MaterialCompilationDisposition::Compiled)
        {
            if (!effectiveAxialModulus || !calibrationMode)
                throw std::invalid_argument(
                    "Compiled MAT-1 response must carry modulus and calibration mode.");
            if (!std::isfinite(*effectiveAxialModulus) || *effectiveAxialModulus <= 0.0)
                throw std::invalid_argument(
                    "Compiled MAT-1 effective axial modulus must be finite and positive.");
        }
        else if (effectiveAxialModulus || calibrationMode)
        {
            throw std::invalid_argument(
                "Uncompiled MAT-1 response must not carry a derived modulus or calibration mode.");
        }

        if (requestedScaleToPeriodRatio
            && (!std::isfinite(*requestedScaleToPeriodRatio)
                || *requestedScaleToPeriodRatio <= 0.0))
            throw std::invalid_argument(
                "MAT-1 requested-scale/period ratio must be finite and positive when present.");
    }

    Mat1AxialCompilationResult Mat1LaminateCalibrationCompiler::compile(
        const Mat1LaminateMaterialState& state,
        const Mat1AxialResponseQuery& query)
    {
        state.validate();
        query.validate();

        std::optional<double> scaleRatio;
        if (query.requestedSpatialScale)
            scaleRatio = *query.requestedSpatialScale / state.laminatePeriod;

        const auto alignment = absoluteDot(state.layerNormal, query.loadingDirection);

        if (alignment <= kDirectionTolerance)
        {
            const auto modulus =
                state.phaseAFraction * state.phaseA.axialElasticModulus
                + state.phaseBFraction() * state.phaseB.axialElasticModulus;
            Mat1AxialCompilationResult result{
                compiledRecord(
                    state,
                    query,
                    Mat1AxialCalibrationMode::InPlaneIsoStrain),
                modulus,
                Mat1AxialCalibrationMode::InPlaneIsoStrain,
                scaleRatio};
            result.validate();
            return result;
        }

        if (1.0 - alignment <= kDirectionTolerance)
        {
            const auto compliance =
                state.phaseAFraction / state.phaseA.axialElasticModulus
                + state.phaseBFraction() / state.phaseB.axialElasticModulus;
            const auto modulus = 1.0 / compliance;
            Mat1AxialCompilationResult result{
                compiledRecord(
                    state,
                    query,
                    Mat1AxialCalibrationMode::LayerNormalIsoStress),
                modulus,
                Mat1AxialCalibrationMode::LayerNormalIsoStress,
                scaleRatio};
            result.validate();
            return result;
        }

        Mat1AxialCompilationResult result{
            unresolvedRecord(state, query),
            std::nullopt,
            std::nullopt,
            scaleRatio};
        result.validate();
        return result;
    }

    const ScientificModelMetadata& Mat1LaminateCalibrationCompiler::scientificMetadata()
    {
        static const auto metadata = makeMetadata();
        return metadata;
    }
} // namespace ae
