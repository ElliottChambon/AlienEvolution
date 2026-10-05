#pragma once

#include <optional>
#include <string>
#include <vector>

#include "alien_evolution/core/CausalDependencyGraph.hpp"
#include "alien_evolution/materials/DevelopedMaterialState.hpp"

namespace ae
{
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
        std::vector<MaterialCompilationCostDescriptor> costs;
        std::optional<std::string> note;

        void validate() const;
    };
} // namespace ae
