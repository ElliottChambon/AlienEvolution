#pragma once

#include <optional>
#include <string>
#include <vector>

#include "alien_evolution/physics/AdaptiveCertifiedPhysics.hpp"

namespace ae
{
    // Stable envelope around a versioned developed-material representation.
    //
    // This deliberately does NOT define a universal payload containing every
    // conceivable composition/phase/microstructure/history variable. Numerical
    // state remains representation-specific and can evolve independently of the
    // envelope as materials science changes.
    struct MaterialStateComponentDescriptor
    {
        std::string key;
        ScientificModelRef representation;
        double representationScale;
        std::optional<std::string> description;

        void validate() const;
    };

    class DevelopedMaterialStateEnvelope
    {
    public:
        DevelopedMaterialStateEnvelope(
            std::string identifier,
            ScientificModelRef stateRepresentation,
            double coarseGrainingLength,
            std::optional<double> characteristicMicrostructureLength,
            std::vector<MaterialStateComponentDescriptor> components);

        [[nodiscard]] const std::string& identifier() const;
        [[nodiscard]] const ScientificModelRef& stateRepresentation() const;
        [[nodiscard]] double coarseGrainingLength() const;
        [[nodiscard]] const std::optional<double>& characteristicMicrostructureLength() const;
        [[nodiscard]] const std::vector<MaterialStateComponentDescriptor>& components() const;
        [[nodiscard]] const MaterialStateComponentDescriptor* findComponent(
            const std::string& key) const;

        void validate() const;

    private:
        std::string identifier_;
        ScientificModelRef stateRepresentation_;
        double coarseGrainingLength_;
        std::optional<double> characteristicMicrostructureLength_;
        std::vector<MaterialStateComponentDescriptor> components_;
    };
} // namespace ae
