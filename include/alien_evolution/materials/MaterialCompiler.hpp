#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "alien_evolution/core/CausalDependencyGraph.hpp"
#include "alien_evolution/materials/DevelopedMaterialState.hpp"

namespace ae
{
    namespace material_update_points
    {
        inline constexpr std::string_view ConstituentPhysics = "constituent_physics";
        inline constexpr std::string_view PhaseThermodynamics = "phase_thermodynamics";
        inline constexpr std::string_view MicrostructureEvolution = "microstructure_evolution";
        inline constexpr std::string_view Homogenization = "homogenization";
        inline constexpr std::string_view ConstitutiveResponse = "constitutive_response";
        inline constexpr std::string_view NumericalRealization = "numerical_realization";
        inline constexpr std::string_view CalibrationReference = "calibration_reference";
    }

    enum class MaterialCompilationDisposition
    {
        Compiled,
        NotHomogenizable,
        UnsupportedQuery,
        ScientificallyUnresolved
    };

    struct MaterialResponseQueryDescriptor
    {
        std::string key;
        std::string responseFamilyIdentifier;
        std::string quantityOfInterestKey;
        std::optional<double> requestedSpatialScale;
        std::optional<double> requestedTemporalScale;
        std::optional<std::string> regimeIdentifier;
        std::optional<std::string> description;

        void validate() const;
    };

    struct MaterialScienceUpdateBinding
    {
        // Open string key by design. M8A uses stable keys such as
        // constituent_physics, homogenization and calibration_reference, but
        // later science may add new update points without changing an enum.
        std::string updatePointKey;
        ScientificModelRef model;
        std::optional<std::string> note;

        void validate() const;
    };

    struct MaterialStateUncertaintyDescriptor
    {
        std::string key;
        std::string stateComponentKey;
        std::string uncertaintyModelIdentifier;
        std::string unit;
        std::optional<std::string> note;

        void validate() const;
    };

    struct MaterialCompilationCostDescriptor
    {
        std::string key;
        double value;
        std::string unit;
        std::optional<std::string> note;

        void validate() const;
    };

    struct MaterialCompilationRecord
    {
        std::string sourceMaterialStateIdentifier;
        MaterialResponseQueryDescriptor query;
        MaterialCompilationDisposition disposition;
        std::optional<ScientificModelRef> compiledModel;
        std::optional<AdaptivePhysicsContract> adaptivePhysicsContract;
        std::vector<MaterialScienceUpdateBinding> updateBindings;
        std::vector<DerivedArtifactId> dependencies;
        // Underlying-state uncertainty is intentionally separate from
        // AdaptivePhysicsContract numerical/reduction error declarations and
        // ScientificModelMetadata model-form uncertainty.
        std::vector<MaterialStateUncertaintyDescriptor> materialStateUncertainties;
        std::vector<MaterialCompilationCostDescriptor> costs;
        std::optional<std::string> note;

        void validate() const;
    };
} // namespace ae
