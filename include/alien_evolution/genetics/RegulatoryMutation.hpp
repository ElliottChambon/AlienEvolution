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

    // V0.2 whole-regulatory-unit duplication.
    //
    // The new node initially inherits the source node's kinetic state
    // and its local incoming/outgoing regulatory relationships.
    //
    // This represents one explicit duplication abstraction and is not
    // intended to imply that every biological duplication event preserves
    // regulatory context in this way.
    [[nodiscard]]
    RegulatoryProgram duplicateRegulatoryUnit(
        const RegulatoryProgram& program,
        std::uint64_t sourceNodeId,
        std::uint64_t newNodeId
    );

    // Removes a regulatory node and every interaction for which that
    // node is either source or target.
    //
    // The final remaining node cannot be deleted because a
    // RegulatoryProgram must contain at least one node.
    [[nodiscard]]
    RegulatoryProgram removeRegulatoryNode(
        const RegulatoryProgram& program,
        std::uint64_t nodeId
    );

} // namespace ae