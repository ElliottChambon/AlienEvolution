#pragma once

#include <cstdint>
#include <variant>
#include <vector>

#include "alien_evolution/genetics/RegulatoryMutation.hpp"
#include "alien_evolution/genetics/RegulatoryProgram.hpp"

namespace ae
{

    struct NodeKineticMutationEvent
    {
        std::uint64_t nodeId = 0;
        NodeKineticChange change{};
    };

    struct InteractionParameterMutationEvent
    {
        std::uint64_t sourceNodeId = 0;
        std::uint64_t targetNodeId = 0;

        InteractionParameterChange change{};
    };

    struct InteractionGainMutationEvent
    {
        RegulatoryInteraction interaction{};
    };

    struct InteractionLossMutationEvent
    {
        std::uint64_t sourceNodeId = 0;
        std::uint64_t targetNodeId = 0;
    };

    struct RegulatoryUnitDuplicationMutationEvent
    {
        std::uint64_t sourceNodeId = 0;
        std::uint64_t newNodeId = 0;
    };

    struct RegulatoryNodeLossMutationEvent
    {
        std::uint64_t nodeId = 0;
    };

    using RegulatoryMutationEvent =
        std::variant<
        NodeKineticMutationEvent,
        InteractionParameterMutationEvent,
        InteractionGainMutationEvent,
        InteractionLossMutationEvent,
        RegulatoryUnitDuplicationMutationEvent,
        RegulatoryNodeLossMutationEvent
        >;

    [[nodiscard]]
    RegulatoryProgram applyRegulatoryMutationEvent(
        const RegulatoryProgram& program,
        const RegulatoryMutationEvent& event
    );

    [[nodiscard]]
    RegulatoryProgram applyRegulatoryMutationEvents(
        const RegulatoryProgram& program,
        const std::vector<RegulatoryMutationEvent>& events
    );

} // namespace ae