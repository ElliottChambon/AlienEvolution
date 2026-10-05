#include "alien_evolution/materials/MaterialCompiler.hpp"

#include <cmath>
#include <stdexcept>
#include <unordered_set>

namespace ae
{
    namespace
    {
        void positiveOptional(
            const std::optional<double>& value,
            const char* message)
        {
            if (value && (!std::isfinite(*value) || *value <= 0.0))
                throw std::invalid_argument(message);
        }
    }

    void MaterialResponseQueryDescriptor::validate() const
    {
        if (key.empty() || responseFamilyIdentifier.empty() || quantityOfInterestKey.empty())
            throw std::invalid_argument(
                "Material response query key, family and quantity-of-interest key must be nonempty.");

        positiveOptional(
            requestedSpatialScale,
            "Requested material-response spatial scale must be finite and positive.");
        positiveOptional(
            requestedTemporalScale,
            "Requested material-response temporal scale must be finite and positive.");

        if (regimeIdentifier && regimeIdentifier->empty())
            throw std::invalid_argument(
                "Material-response regime identifier must be nonempty when present.");
        if (description && description->empty())
            throw std::invalid_argument(
                "Material-response query description must be nonempty when present.");
    }

    void MaterialScienceUpdateBinding::validate() const
    {
        if (updatePointKey.empty())
            throw std::invalid_argument("Material science update-point key must not be empty.");
        model.validate();
        if (note && note->empty())
            throw std::invalid_argument(
                "Material science update-point note must be nonempty when present.");
    }

    void MaterialCompilationCostDescriptor::validate() const
    {
        if (key.empty() || unit.empty())
            throw std::invalid_argument(
                "Material compilation cost key and unit must be nonempty.");
        if (!std::isfinite(value) || value < 0.0)
            throw std::invalid_argument(
                "Material compilation cost value must be finite and nonnegative.");
        if (note && note->empty())
            throw std::invalid_argument(
                "Material compilation cost note must be nonempty when present.");
    }

    void MaterialCompilationRecord::validate() const
    {
        if (sourceMaterialStateIdentifier.empty())
            throw std::invalid_argument(
                "Material compilation source-state identifier must not be empty.");
        query.validate();

        if (disposition == MaterialCompilationDisposition::Compiled)
        {
            if (!compiledModel || !adaptivePhysicsContract)
                throw std::invalid_argument(
                    "Compiled material response requires a compiled model and ACP contract.");
            compiledModel->validate();
            adaptivePhysicsContract->validate();
            if (!(adaptivePhysicsContract->model == *compiledModel))
                throw std::invalid_argument(
                    "Material compilation ACP contract must identify the compiled model.");
        }
        else if (compiledModel || adaptivePhysicsContract)
        {
            throw std::invalid_argument(
                "Uncompiled material response must not carry a compiled model or ACP contract.");
        }

        std::unordered_set<std::string> updateKeys;
        updateKeys.reserve(updateBindings.size());
        for (const auto& binding : updateBindings)
        {
            binding.validate();
            if (!updateKeys.insert(binding.updatePointKey).second)
                throw std::invalid_argument(
                    "Material science update-point keys must be unique within a compilation record.");
        }

        std::unordered_set<std::string> dependencyKeys;
        dependencyKeys.reserve(dependencies.size());
        for (const auto& dependency : dependencies)
        {
            dependency.validate();
            if (!dependencyKeys.insert(dependency.value()).second)
                throw std::invalid_argument(
                    "Material compilation dependency identifiers must be unique.");
        }

        std::unordered_set<std::string> costKeys;
        costKeys.reserve(costs.size());
        for (const auto& cost : costs)
        {
            cost.validate();
            if (!costKeys.insert(cost.key).second)
                throw std::invalid_argument(
                    "Material compilation cost keys must be unique.");
        }

        if (note && note->empty())
            throw std::invalid_argument(
                "Material compilation note must be nonempty when present.");
    }
} // namespace ae
