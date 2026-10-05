#include "alien_evolution/materials/DevelopedMaterialState.hpp"

#include <cmath>
#include <stdexcept>
#include <unordered_set>

namespace ae
{
    void MaterialStateComponentDescriptor::validate() const
    {
        if (key.empty())
            throw std::invalid_argument("Material-state component key must not be empty.");
        representation.validate();
        if (!std::isfinite(representationScale) || representationScale <= 0.0)
            throw std::invalid_argument(
                "Material-state component representation scale must be finite and positive.");
        if (description && description->empty())
            throw std::invalid_argument(
                "Material-state component description must be nonempty when present.");
    }

    DevelopedMaterialStateEnvelope::DevelopedMaterialStateEnvelope(
        std::string identifier,
        ScientificModelRef stateRepresentation,
        double coarseGrainingLength,
        std::optional<double> characteristicMicrostructureLength,
        std::vector<MaterialStateComponentDescriptor> components)
        : identifier_(std::move(identifier)),
          stateRepresentation_(std::move(stateRepresentation)),
          coarseGrainingLength_(coarseGrainingLength),
          characteristicMicrostructureLength_(characteristicMicrostructureLength),
          components_(std::move(components))
    {
        validate();
    }

    const std::string& DevelopedMaterialStateEnvelope::identifier() const
    {
        return identifier_;
    }

    const ScientificModelRef& DevelopedMaterialStateEnvelope::stateRepresentation() const
    {
        return stateRepresentation_;
    }

    double DevelopedMaterialStateEnvelope::coarseGrainingLength() const
    {
        return coarseGrainingLength_;
    }

    const std::optional<double>&
        DevelopedMaterialStateEnvelope::characteristicMicrostructureLength() const
    {
        return characteristicMicrostructureLength_;
    }

    const std::vector<MaterialStateComponentDescriptor>&
        DevelopedMaterialStateEnvelope::components() const
    {
        return components_;
    }

    const MaterialStateComponentDescriptor*
        DevelopedMaterialStateEnvelope::findComponent(const std::string& key) const
    {
        for (const auto& component : components_)
            if (component.key == key) return &component;
        return nullptr;
    }

    void DevelopedMaterialStateEnvelope::validate() const
    {
        if (identifier_.empty())
            throw std::invalid_argument("Developed material-state identifier must not be empty.");
        stateRepresentation_.validate();

        if (!std::isfinite(coarseGrainingLength_) || coarseGrainingLength_ <= 0.0)
            throw std::invalid_argument(
                "Developed material-state coarse-graining length must be finite and positive.");

        if (characteristicMicrostructureLength_
            && (!std::isfinite(*characteristicMicrostructureLength_)
                || *characteristicMicrostructureLength_ <= 0.0))
            throw std::invalid_argument(
                "Characteristic microstructure length must be finite and positive when present.");

        std::unordered_set<std::string> keys;
        keys.reserve(components_.size());
        for (const auto& component : components_)
        {
            component.validate();
            if (!keys.insert(component.key).second)
                throw std::invalid_argument(
                    "Developed material-state component keys must be unique.");
        }
    }
} // namespace ae
