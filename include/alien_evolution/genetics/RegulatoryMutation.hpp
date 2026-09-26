#pragma once

#include <cstddef>
#include <cstdint>

#include "alien_evolution/genetics/RegulatoryProgram.hpp"

namespace ae
{

    struct NodeKineticChange
    {
        double basalProductionMultiplier = 1.0;
        double degradationMultiplier = 1.0;
    };

    struct InteractionParameterChange
    {
        double foldChangeMultiplier = 1.0;
        double halfSaturationMultiplier = 1.0;
        double cooperativityMultiplier = 1.0;
    };

    [[nodiscard]]
    RegulatoryProgram changeNodeKinetics(
        const RegulatoryProgram& program,
        std::uint64_t nodeId,
        const NodeKineticChange& change
    );

    [[nodiscard]]
    RegulatoryProgram changeInteractionParameters(
        const RegulatoryProgram& program,
        std::size_t interactionIndex,
        const InteractionParameterChange& change
    );

    [[nodiscard]]
    RegulatoryProgram addRegulatoryInteraction(
        const RegulatoryProgram& program,
        const RegulatoryInteraction& interaction
    );

    [[nodiscard]]
    RegulatoryProgram removeRegulatoryInteraction(
        const RegulatoryProgram& program,
        std::size_t interactionIndex
    );

} // namespace ae