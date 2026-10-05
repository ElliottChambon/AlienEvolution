#include "alien_evolution/materials/Mat1LaminateCalibration.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace
{
    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    void nearRelative(double actual, double expected, double tolerance = 2.0e-12)
    {
        require(std::isfinite(actual) && std::isfinite(expected), "Nonfinite comparison");
        const auto scale = std::max({1.0, std::abs(actual), std::abs(expected)});
        require(std::abs(actual - expected) <= tolerance * scale,
            "Relative numerical comparison failed");
    }

    template <class Exception = std::invalid_argument, class Function>
    void rejects(Function function)
    {
        try { function(); }
        catch (const Exception&) { return; }
        throw std::runtime_error("Expected exception was not thrown");
    }

    ae::Mat1LaminateMaterialState laminate(
        double ea = 100.0,
        double eb = 10.0,
        double fa = 0.25,
        std::array<double, 3> normal = {1.0, 0.0, 0.0})
    {
        return {
            "material.mat1.fixture",
            {"phase-A", ea},
            {"phase-B", eb},
            fa,
            0.01,
            normal};
    }

    void developedStateEnvelope()
    {
        ae::DevelopedMaterialStateEnvelope state(
            "material.fixture",
            ae::ScientificModelRef("alien_evolution.material.fixture", "v1"),
            0.1,
            0.01,
            {
                {"composition",
                    ae::ScientificModelRef("alien_evolution.material.fixture.composition", "v1"),
                    0.01,
                    "Fixture composition"},
                {"microstructure",
                    ae::ScientificModelRef("alien_evolution.material.fixture.microstructure", "v2"),
                    0.005,
                    "Fixture microstructure"}
            });

        state.validate();
        require(state.identifier() == "material.fixture", "Material-state identifier changed");
        require(state.coarseGrainingLength() == 0.1, "Material-state scale changed");
        require(state.characteristicMicrostructureLength()
            && *state.characteristicMicrostructureLength() == 0.01,
            "Material-state microstructure scale changed");
        require(state.components().size() == 2
            && state.components()[0].key == "composition"
            && state.components()[1].key == "microstructure",
            "Material-state component order changed");
        require(state.findComponent("microstructure") != nullptr
            && state.findComponent("missing") == nullptr,
            "Material-state component lookup changed");

        rejects([] {
            (void)ae::DevelopedMaterialStateEnvelope(
                "bad",
                ae::ScientificModelRef("alien_evolution.material.bad", "v1"),
                0.0,
                std::nullopt,
                {});
        });
        rejects([] {
            (void)ae::DevelopedMaterialStateEnvelope(
                "bad",
                ae::ScientificModelRef("alien_evolution.material.bad", "v1"),
                1.0,
                -1.0,
                {});
        });
        rejects([] {
            (void)ae::DevelopedMaterialStateEnvelope(
                "bad",
                ae::ScientificModelRef("alien_evolution.material.bad", "v1"),
                1.0,
                std::nullopt,
                {
                    {"same", ae::ScientificModelRef("m.a", "v1"), 1.0, std::nullopt},
                    {"same", ae::ScientificModelRef("m.b", "v1"), 1.0, std::nullopt}
                });
        });
    }

    void compilerVocabulary()
    {
        ae::MaterialResponseQueryDescriptor query{
            "q",
            "mechanical.fixture",
            "fixture_qoi",
            1.0,
            2.0,
            "fixture_regime",
            "Fixture query"};
        query.validate();

        ae::MaterialScienceUpdateBinding custom{
            "future_science_stage_not_in_m8a_enum",
            ae::ScientificModelRef("alien_evolution.material.future", "v7"),
            "Open string update point remains extensible"};
        custom.validate();

        ae::AdaptivePhysicsContract contract{
            ae::ScientificModelRef("compiled.fixture", "v1"),
            {{"fixture_qoi", "fixture.qoi", "Pa", "fixture"}},
            {},
            {},
            {},
            "fixture"};
        contract.validate();

        ae::MaterialCompilationRecord compiled{
            "material.fixture",
            query,
            ae::MaterialCompilationDisposition::Compiled,
            ae::ScientificModelRef("compiled.fixture", "v1"),
            contract,
            {custom},
            {ae::DerivedArtifactId("material.fixture")},
            {{"composition_measurement", "composition", "fixture.interval", "dimensionless",
                "Underlying state uncertainty remains separate"}},
            {{"cpu_estimate", 2.0, "arbitrary_cost_unit", "Simulator metadata"}},
            "fixture"};
        compiled.validate();
        require(compiled.materialStateUncertainties.size() == 1
            && compiled.materialStateUncertainties[0].stateComponentKey == "composition",
            "Material-state uncertainty was not retained separately");

        rejects([&] {
            auto bad = compiled;
            bad.updateBindings.clear();
            bad.validate();
        });
        rejects([&] {
            auto bad = compiled;
            bad.dependencies.clear();
            bad.validate();
        });
        rejects([&] {
            auto bad = compiled;
            bad.adaptivePhysicsContract->quantitiesOfInterest.clear();
            bad.validate();
        });

        ae::MaterialCompilationRecord unresolved{
            "material.fixture",
            query,
            ae::MaterialCompilationDisposition::NotHomogenizable,
            std::nullopt,
            std::nullopt,
            {},
            {ae::DerivedArtifactId("material.fixture")},
            {},
            {},
            "Compiler is allowed to refuse homogenization"};
        unresolved.validate();

        rejects([&] {
            auto bad = unresolved;
            bad.compiledModel = ae::ScientificModelRef("fake", "v1");
            bad.validate();
        });

        rejects([&] {
            auto bad = compiled;
            bad.updateBindings.push_back(custom);
            bad.validate();
        });

        rejects([&] {
            auto bad = compiled;
            bad.costs.push_back({"cpu_estimate", 3.0, "unit", std::nullopt});
            bad.validate();
        });
    }

    void mat1ExactResponses()
    {
        const auto state = laminate();

        const ae::Mat1AxialResponseQuery inPlane({0.0, 1.0, 0.0}, 1.0);
        const ae::Mat1AxialResponseQuery normal({1.0, 0.0, 0.0}, 1.0);

        const auto parallel =
            ae::Mat1LaminateCalibrationCompiler::compile(state, inPlane);
        const auto perpendicular =
            ae::Mat1LaminateCalibrationCompiler::compile(state, normal);

        require(parallel.record.disposition == ae::MaterialCompilationDisposition::Compiled
            && parallel.calibrationMode
            == ae::Mat1AxialCalibrationMode::InPlaneIsoStrain,
            "MAT-1 in-plane iso-strain classification changed");
        require(perpendicular.record.disposition == ae::MaterialCompilationDisposition::Compiled
            && perpendicular.calibrationMode
            == ae::Mat1AxialCalibrationMode::LayerNormalIsoStress,
            "MAT-1 layer-normal iso-stress classification changed");

        const double expectedArithmetic = 0.25 * 100.0 + 0.75 * 10.0;
        const double expectedHarmonic =
            1.0 / (0.25 / 100.0 + 0.75 / 10.0);
        nearRelative(*parallel.effectiveAxialModulus, expectedArithmetic);
        nearRelative(*perpendicular.effectiveAxialModulus, expectedHarmonic);
        require(*perpendicular.effectiveAxialModulus <= *parallel.effectiveAxialModulus,
            "MAT-1 harmonic response exceeded arithmetic response");

        require(parallel.requestedScaleToPeriodRatio
            && *parallel.requestedScaleToPeriodRatio == 100.0,
            "MAT-1 scale/period diagnostic changed");
        require(parallel.record.note
            && parallel.record.note->find("not certified") != std::string::npos,
            "MAT-1 finite-scale representativity guardrail missing");
    }

    void mat1LimitsAndSymmetries()
    {
        const ae::Mat1AxialResponseQuery inPlane({0.0, 2.0, 0.0});
        const ae::Mat1AxialResponseQuery normal({3.0, 0.0, 0.0});

        for (const auto fraction : {0.0, 1.0})
        {
            const auto state = laminate(100.0, 10.0, fraction);
            const auto expected = fraction == 1.0 ? 100.0 : 10.0;
            nearRelative(
                *ae::Mat1LaminateCalibrationCompiler::compile(state, inPlane)
                     .effectiveAxialModulus,
                expected);
            nearRelative(
                *ae::Mat1LaminateCalibrationCompiler::compile(state, normal)
                     .effectiveAxialModulus,
                expected);
        }

        const auto equal = laminate(42.0, 42.0, 0.37);
        nearRelative(
            *ae::Mat1LaminateCalibrationCompiler::compile(equal, inPlane)
                 .effectiveAxialModulus,
            42.0);
        nearRelative(
            *ae::Mat1LaminateCalibrationCompiler::compile(equal, normal)
                 .effectiveAxialModulus,
            42.0);

        const auto original = laminate(100.0, 10.0, 0.25);
        ae::Mat1LaminateMaterialState swapped(
            "material.mat1.fixture",
            {"phase-B", 10.0},
            {"phase-A", 100.0},
            0.75,
            0.01,
            {1.0, 0.0, 0.0});

        nearRelative(
            *ae::Mat1LaminateCalibrationCompiler::compile(original, inPlane)
                 .effectiveAxialModulus,
            *ae::Mat1LaminateCalibrationCompiler::compile(swapped, inPlane)
                 .effectiveAxialModulus);
        nearRelative(
            *ae::Mat1LaminateCalibrationCompiler::compile(original, normal)
                 .effectiveAxialModulus,
            *ae::Mat1LaminateCalibrationCompiler::compile(swapped, normal)
                 .effectiveAxialModulus);

        const double scale = 7.0;
        const auto scaled = laminate(700.0, 70.0, 0.25);
        nearRelative(
            *ae::Mat1LaminateCalibrationCompiler::compile(scaled, inPlane)
                 .effectiveAxialModulus,
            scale
                * *ae::Mat1LaminateCalibrationCompiler::compile(original, inPlane)
                      .effectiveAxialModulus);
        nearRelative(
            *ae::Mat1LaminateCalibrationCompiler::compile(scaled, normal)
                 .effectiveAxialModulus,
            scale
                * *ae::Mat1LaminateCalibrationCompiler::compile(original, normal)
                      .effectiveAxialModulus);
    }

    void mat1OrientationAndScope()
    {
        const auto state = laminate(
            100.0,
            10.0,
            0.25,
            {2.0, 0.0, 0.0});

        const ae::Mat1AxialResponseQuery oblique({1.0, 1.0, 0.0}, 0.02);
        const auto result =
            ae::Mat1LaminateCalibrationCompiler::compile(state, oblique);

        require(result.record.disposition
            == ae::MaterialCompilationDisposition::UnsupportedQuery,
            "MAT-1 invented an oblique effective response");
        require(!result.effectiveAxialModulus && !result.calibrationMode,
            "MAT-1 unsupported query carried a fake compiled property");
        require(result.requestedScaleToPeriodRatio
            && *result.requestedScaleToPeriodRatio == 2.0,
            "MAT-1 unsupported query lost scale context");

        rejects([] {
            (void)ae::Mat1AxialResponseQuery({0.0, 0.0, 0.0});
        });

        const ae::Mat1AxialResponseQuery hugeDirection(
            {1.0e308, 1.0e308, 0.0});
        require(std::isfinite(hugeDirection.loadingDirection[0])
            && std::isfinite(hugeDirection.loadingDirection[1]),
            "MAT-1 finite large direction failed overflow-safe normalization");
        rejects([] {
            (void)laminate(0.0, 10.0, 0.5);
        });
        rejects([] {
            (void)laminate(100.0, 10.0, -0.1);
        });

        const auto& metadata =
            ae::Mat1LaminateCalibrationCompiler::scientificMetadata();
        metadata.validate();
        require(metadata.identifier == "alien_evolution.mat1.axial_compiled_response"
            && metadata.version == "mat1-v1",
            "MAT-1 scientific identity changed");
        require(metadata.validityScope.find("full tensor elasticity") != std::string::npos,
            "MAT-1 full-3D limitation missing");
        require(std::any_of(
            metadata.assumptions.begin(),
            metadata.assumptions.end(),
            [](const std::string& value)
            {
                return value.find("No finite-scale RVE certification") != std::string::npos;
            }),
            "MAT-1 finite-scale RVE guardrail missing");

        const auto compiled =
            ae::Mat1LaminateCalibrationCompiler::compile(
                state,
                ae::Mat1AxialResponseQuery({0.0, 1.0, 0.0}));
        require(compiled.record.updateBindings.size() == 5,
            "MAT-1 scientific update-point bindings changed");
        require(std::none_of(
            compiled.record.updateBindings.begin(),
            compiled.record.updateBindings.end(),
            [](const auto& binding)
            {
                return binding.updatePointKey == "phase_thermodynamics"
                    || binding.updatePointKey == "microstructure_evolution";
            }),
            "MAT-1 pretended to use scientific update stages absent from the benchmark");
        require(compiled.record.dependencies.size() == 1
            && compiled.record.dependencies[0].value() == state.envelope.identifier(),
            "MAT-1 dependency/invalidation provenance changed");
    }
}

int main()
{
    try
    {
        developedStateEnvelope();
        compilerVocabulary();
        mat1ExactResponses();
        mat1LimitsAndSymmetries();
        mat1OrientationAndScope();
        std::cout << "M8A material-state/compiler tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
