#include "alien_evolution/materials/Mat1LaminateCalibration.hpp"
#include "alien_evolution/materials/Mat2LaminateCalibration.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <vector>

namespace
{
    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    void nearRelative(
        double actual,
        double expected,
        double tolerance = 2.0e-10)
    {
        require(std::isfinite(actual) && std::isfinite(expected), "Nonfinite comparison");
        const auto scale = std::max({1.0, std::abs(actual), std::abs(expected)});
        require(std::abs(actual - expected) <= tolerance * scale,
            "Relative numerical comparison failed");
    }

    void tensorNear(
        const ae::SymmetricTensor3& actual,
        const ae::SymmetricTensor3& expected,
        double tolerance = 2.0e-10)
    {
        nearRelative(actual.xx, expected.xx, tolerance);
        nearRelative(actual.yy, expected.yy, tolerance);
        nearRelative(actual.zz, expected.zz, tolerance);
        nearRelative(actual.yz, expected.yz, tolerance);
        nearRelative(actual.xz, expected.xz, tolerance);
        nearRelative(actual.xy, expected.xy, tolerance);
    }

    template <class Exception = std::invalid_argument, class Function>
    void rejects(Function function)
    {
        try { function(); }
        catch (const Exception&) { return; }
        throw std::runtime_error("Expected exception was not thrown");
    }

    ae::Mat2LaminateMaterialState laminate(
        double kA = 100.0,
        double gA = 40.0,
        double kB = 10.0,
        double gB = 4.0,
        double fA = 0.3,
        std::array<double, 3> normal = {0.0, 0.0, 1.0})
    {
        return {
            "material.mat2.fixture",
            {"phase-A", {kA, gA}},
            {"phase-B", {kB, gB}},
            fA,
            0.01,
            normal};
    }

    void kelvinTensorAndRotation()
    {
        const ae::SymmetricTensor3 value{
            1.0, -2.0, 3.0,
            0.5, -0.75, 1.25};

        const auto kelvin = value.kelvin();
        const auto roundTrip = ae::SymmetricTensor3::fromKelvin(kelvin);
        tensorNear(roundTrip, value, 1.0e-13);

        double kelvinInner = 0.0;
        for (const auto component : kelvin)
            kelvinInner += component * component;
        nearRelative(value.inner(value), kelvinInner, 1.0e-13);

        const ae::SymmetricTensor3 pureShear{
            0.0, 0.0, 0.0,
            2.0, 0.0, 0.0};
        nearRelative(
            pureShear.kelvin()[3],
            2.0 * std::numbers::sqrt2,
            1.0e-13);
        nearRelative(
            pureShear.norm(),
            2.0 * std::numbers::sqrt2,
            1.0e-13);

        const auto rotation =
            ae::Rotation3::fromAxisAngle({1.0, 2.0, 3.0}, 0.73);
        nearRelative(
            value.rotated(rotation).norm(),
            value.norm(),
            2.0e-12);

        rejects([] {
            (void)ae::Rotation3({
                1.0, 0.0, 0.0,
                0.0, 1.0, 0.0,
                0.0, 0.0, -1.0});
        });
    }

    void isotropicElasticity()
    {
        const ae::IsotropicLinearElasticity material{100.0, 40.0};
        const auto tensor = material.tensor();

        const auto volumetric =
            ae::SymmetricTensor3::identity(0.01);
        const auto volumetricStress = tensor.stress(volumetric);
        const auto expectedHydrostatic = 100.0 * 0.03;
        tensorNear(
            volumetricStress,
            ae::SymmetricTensor3::identity(expectedHydrostatic),
            2.0e-12);
        nearRelative(
            tensor.energyDensity(volumetric),
            0.5 * 100.0 * 0.03 * 0.03,
            2.0e-12);

        const ae::SymmetricTensor3 deviator{
            0.01, -0.01, 0.0,
            0.002, 0.0, 0.0};
        require(std::abs(deviator.trace()) < 1.0e-15, "Fixture is not deviatoric");
        tensorNear(
            tensor.stress(deviator),
            deviator * 80.0,
            2.0e-12);

        nearRelative(
            material.youngModulus(),
            9.0 * 100.0 * 40.0 / 340.0,
            2.0e-12);
        nearRelative(
            material.poissonRatio(),
            (300.0 - 80.0) / (2.0 * 340.0),
            2.0e-12);

        const ae::IsotropicLinearElasticity auxetic{1.0, 3.0};
        require(auxetic.poissonRatio() < 0.0, "Positive K/G should permit auxetic benchmark state");

        const ae::IsotropicLinearElasticity huge{1.0e150, 2.0e150};
        require(std::isfinite(huge.youngModulus())
            && std::isfinite(huge.poissonRatio()),
            "Large finite K/G should retain representable derived elastic constants");
        huge.tensor().validate();

        const ae::IsotropicLinearElasticity tiny{1.0e-150, 2.0e-150};
        require(std::isfinite(tiny.youngModulus())
            && tiny.youngModulus() > 0.0,
            "Tiny finite K/G should retain positive derived elastic constants");
        tiny.tensor().validate();
        require(
            !huge.tensor().approximatelyEqual(tiny.tensor(), 1.0e-12),
            "Scale-aware tensor comparison collapsed huge and tiny stiffness states");

        const auto rotation =
            ae::Rotation3::fromAxisAngle({2.0, -1.0, 0.5}, 1.1);
        require(
            tensor.rotated(rotation).approximatelyEqual(tensor, 2.0e-11),
            "Isotropic elasticity changed under proper rotation");

        const auto rotatedStrain = deviator.rotated(rotation);
        nearRelative(
            tensor.energyDensity(rotatedStrain),
            tensor.energyDensity(deviator),
            2.0e-11);

        rejects([] {
            (void)ae::IsotropicLinearElasticity{0.0, 1.0}.tensor();
        });
        rejects([] {
            (void)ae::IsotropicLinearElasticity{1.0, -1.0}.tensor();
        });
    }

    void mat2LimitsSymmetryAndRotation()
    {
        const auto query = ae::Mat2ElasticResponseQuery{1.0};
        const auto base = laminate();
        const auto compiled =
            ae::Mat2LaminateCalibrationCompiler::compile(base, query);

        require(compiled.record.disposition
            == ae::MaterialCompilationDisposition::Compiled,
            "MAT-2 did not return compiled response");
        require(compiled.requestedScaleToPeriodRatio
            && *compiled.requestedScaleToPeriodRatio == 100.0,
            "MAT-2 scale/period diagnostic changed");

        const auto phaseA = base.phaseA.elasticity.tensor();
        const auto phaseB = base.phaseB.elasticity.tensor();

        require(
            ae::Mat2LaminateCalibrationCompiler::compile(
                laminate(100.0, 40.0, 10.0, 4.0, 1.0))
                .effectiveTensor.approximatelyEqual(phaseA, 2.0e-11),
            "MAT-2 pure-A limit failed");
        require(
            ae::Mat2LaminateCalibrationCompiler::compile(
                laminate(100.0, 40.0, 10.0, 4.0, 0.0))
                .effectiveTensor.approximatelyEqual(phaseB, 2.0e-11),
            "MAT-2 pure-B limit failed");

        const auto identical =
            ae::Mat2LaminateCalibrationCompiler::compile(
                laminate(25.0, 7.0, 25.0, 7.0, 0.37));
        require(
            identical.effectiveTensor.approximatelyEqual(
                ae::IsotropicLinearElasticity{25.0, 7.0}.tensor(),
                2.0e-11),
            "MAT-2 identical-phase limit failed");

        const auto swapped =
            ae::Mat2LaminateCalibrationCompiler::compile(
                laminate(10.0, 4.0, 100.0, 40.0, 0.7));
        require(
            compiled.effectiveTensor.approximatelyEqual(
                swapped.effectiveTensor,
                2.0e-11),
            "MAT-2 phase-label/fraction exchange invariance failed");

        // Local z-normal tensor should be transversely isotropic:
        // any rotation about z leaves it unchanged.
        const auto aboutNormal =
            ae::Rotation3::fromAxisAngle({0.0, 0.0, 1.0}, 0.61);
        require(
            compiled.effectiveTensor.rotated(aboutNormal)
                .approximatelyEqual(compiled.effectiveTensor, 3.0e-10),
            "MAT-2 laminate is not invariant under rotation about layer normal");

        // But a generic unequal-phase laminate should not be fully isotropic.
        const auto tipNormal =
            ae::Rotation3::fromAxisAngle({0.0, 1.0, 0.0}, 0.5 * std::numbers::pi);
        require(
            !compiled.effectiveTensor.rotated(tipNormal)
                .approximatelyEqual(compiled.effectiveTensor, 1.0e-6),
            "MAT-2 unequal-phase laminate was falsely isotropic");

        const std::array<double, 3> newNormal{1.0, 2.0, 3.0};
        const auto rotatedState =
            ae::Mat2LaminateCalibrationCompiler::compile(
                laminate(100.0, 40.0, 10.0, 4.0, 0.3, newNormal));
        const auto localToGlobal =
            ae::Rotation3::fromLayerNormal(newNormal);
        require(
            rotatedState.effectiveTensor.approximatelyEqual(
                compiled.effectiveTensor.rotated(localToGlobal),
                4.0e-10),
            "MAT-2 layer-normal rotation did not rotate Ceff covariantly");

        const ae::SymmetricTensor3 strain{
            0.01, -0.004, 0.003,
            0.002, -0.001, 0.0005};
        nearRelative(
            rotatedState.effectiveTensor.energyDensity(
                strain.rotated(localToGlobal)),
            compiled.effectiveTensor.energyDensity(strain),
            5.0e-10);
    }

    void mat2HillMandelAndBounds()
    {
        const auto state = laminate();
        const auto compiled =
            ae::Mat2LaminateCalibrationCompiler::compile(state);

        const std::vector<ae::SymmetricTensor3> strains{
            {0.01, -0.004, 0.003, 0.002, -0.001, 0.0005},
            {0.0, 0.0, 0.01, 0.0, 0.0, 0.0},
            {0.01, 0.01, 0.01, 0.0, 0.0, 0.0},
            {0.0, 0.0, 0.0, 0.005, 0.002, -0.003}};

        for (const auto& strain : strains)
        {
            const auto response =
                ae::Mat2LaminateCalibrationCompiler::evaluateMicroscale(
                    state,
                    strain);

            tensorNear(
                response.volumeAverageMicroStrain,
                strain,
                2.0e-10);
            tensorNear(
                response.volumeAverageMicroStress,
                response.macroStress,
                3.0e-10);

            const auto energyScale =
                std::max(1.0, std::abs(response.macroElasticEnergyDensity));
            require(
                std::abs(response.energyResidual) <= 5.0e-10 * energyScale,
                "MAT-2 micro/macro elastic energy mismatch");
            require(
                std::abs(response.hillMandelWorkResidual)
                    <= 1.0e-9 * energyScale,
                "MAT-2 Hill-Mandel work residual too large");

            const auto reussEnergy =
                compiled.reussLowerBound.energyDensity(strain);
            const auto effectiveEnergy =
                compiled.effectiveTensor.energyDensity(strain);
            const auto voigtEnergy =
                compiled.voigtUpperBound.energyDensity(strain);

            require(
                reussEnergy <= effectiveEnergy + 2.0e-10 * std::max(1.0, effectiveEnergy),
                "MAT-2 effective energy fell below Reuss tensor bound");
            require(
                effectiveEnergy <= voigtEnergy + 2.0e-10 * std::max(1.0, voigtEnergy),
                "MAT-2 effective energy exceeded Voigt tensor bound");
        }
    }

    void mat1CompatibleSpecialCase()
    {
        // For isotropic phases with lambda=0 (nu=0), axial normal components
        // decouple. K=E/3 and G=E/2 therefore makes the true 3-D MAT-2
        // in-plane constrained-strain and layer-normal series responses reduce
        // exactly to MAT-1's scalar arithmetic/harmonic calibration.
        const double eA = 100.0;
        const double eB = 10.0;
        const double fA = 0.25;

        const auto mat2 =
            ae::Mat2LaminateCalibrationCompiler::compile(
                laminate(eA / 3.0, eA / 2.0, eB / 3.0, eB / 2.0, fA));

        const auto mat1 = ae::Mat1LaminateMaterialState(
            "material.mat1.compatibility",
            {"phase-A", eA},
            {"phase-B", eB},
            fA,
            0.01,
            {0.0, 0.0, 1.0});

        const auto mat1InPlane =
            ae::Mat1LaminateCalibrationCompiler::compile(
                mat1,
                ae::Mat1AxialResponseQuery({1.0, 0.0, 0.0}));
        const auto mat1Normal =
            ae::Mat1LaminateCalibrationCompiler::compile(
                mat1,
                ae::Mat1AxialResponseQuery({0.0, 0.0, 1.0}));

        nearRelative(
            mat2.effectiveTensor.at(0, 0),
            *mat1InPlane.effectiveAxialModulus,
            2.0e-10);
        nearRelative(
            mat2.effectiveTensor.at(2, 2),
            *mat1Normal.effectiveAxialModulus,
            2.0e-10);
    }

    void challengeRegimesAndMetadata()
    {
        // High contrast + near incompressibility.
        const auto hard =
            ae::Mat2LaminateCalibrationCompiler::compile(
                laminate(
                    1.0e6, 1.0,
                    1.0, 1.0e-2,
                    0.4,
                    {2.0, -1.0, 3.0}));
        hard.effectiveTensor.validate();

        const auto response =
            ae::Mat2LaminateCalibrationCompiler::evaluateMicroscale(
                laminate(
                    1.0e6, 1.0,
                    1.0, 1.0e-2,
                    0.4,
                    {2.0, -1.0, 3.0}),
                {1.0e-5, -2.0e-5, 1.0e-5, 2.0e-6, -1.0e-6, 3.0e-6});
        require(std::isfinite(response.hillMandelWorkResidual),
            "MAT-2 high-contrast challenge produced nonfinite residual");
        const auto challengeScale =
            std::max(1.0, std::abs(response.macroElasticEnergyDensity));
        require(
            std::abs(response.hillMandelWorkResidual)
                <= 2.0e-7 * challengeScale
            && std::abs(response.energyResidual)
                <= 1.0e-7 * challengeScale,
            "MAT-2 high-contrast challenge lost micro/macro energy consistency");

        // Auxetic constituent remains physically accepted through positive K/G.
        const auto auxetic =
            ae::Mat2LaminateCalibrationCompiler::compile(
                laminate(1.0, 3.0, 5.0, 2.0, 0.5));
        auxetic.effectiveTensor.validate();

        const auto& metadata =
            ae::Mat2LaminateCalibrationCompiler::scientificMetadata();
        metadata.validate();
        require(metadata.identifier == "alien_evolution.mat2.elasticity_operator"
            && metadata.version == "mat2-v1",
            "MAT-2 scientific identity changed");
        require(metadata.validityScope.find("Structural equilibrium") != std::string::npos,
            "MAT-2 structural-mechanics non-goal missing");

        const auto compiled =
            ae::Mat2LaminateCalibrationCompiler::compile(laminate());
        require(compiled.record.updateBindings.size() == 5,
            "MAT-2 scientific update bindings changed");
        require(std::none_of(
            compiled.record.updateBindings.begin(),
            compiled.record.updateBindings.end(),
            [](const auto& binding)
            {
                return binding.updatePointKey
                    == ae::material_update_points::PhaseThermodynamics
                    || binding.updatePointKey
                    == ae::material_update_points::MicrostructureEvolution;
            }),
            "MAT-2 claimed unused scientific update stages");
        require(compiled.record.note
            && compiled.record.note->find("finite-scale") != std::string::npos,
            "MAT-2 finite-scale RVE guardrail missing");

        rejects([] {
            (void)laminate(1.0, 1.0, 2.0, 2.0, -0.1);
        });
        rejects([] {
            (void)laminate(1.0, 1.0, 2.0, 2.0, 0.5, {0.0, 0.0, 0.0});
        });
        rejects([] {
            auto inconsistent = laminate();
            inconsistent.layerNormal = {0.0, 0.0, 2.0};
            inconsistent.validate();
        });
        rejects([] {
            auto inconsistent = laminate();
            inconsistent.laminatePeriod = 0.02;
            inconsistent.validate();
        });
    }
}

int main()
{
    try
    {
        kelvinTensorAndRotation();
        isotropicElasticity();
        mat2LimitsSymmetryAndRotation();
        mat2HillMandelAndBounds();
        mat1CompatibleSpecialCase();
        challengeRegimesAndMetadata();
        std::cout << "M8B MAT-2 elasticity/homogenization tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
