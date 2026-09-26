#include "alien_evolution/genetics/RegulatoryMutationEvent.hpp"

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace ae
{

    namespace
    {

        std::size_t findInteractionIndex(
            const RegulatoryProgram& program,
            const std::uint64_t sourceNodeId,
            const std::uint64_t targetNodeId
        )
        {
            const auto& interactions =
                program.interactions();

            for (std::size_t i = 0;
                i < interactions.size();
                ++i)
            {
                if (
                    interactions[i].sourceNodeId
                    == sourceNodeId
                    && interactions[i].targetNodeId
                    == targetNodeId
                    )
                {
                    return i;
                }
            }

            throw std::out_of_range(
                "Requested regulatory interaction does not exist."
            );
        }

    } // namespace

    RegulatoryProgram applyRegulatoryMutationEvent(
        const RegulatoryProgram& program,
        const RegulatoryMutationEvent& event
    )
    {
        return std::visit(
            [&program](const auto& mutation)
            -> RegulatoryProgram
            {
                using MutationType =
                    std::decay_t<
                    decltype(mutation)
                    >;

                if constexpr (
                    std::is_same_v<
                    MutationType,
                    NodeKineticMutationEvent
                    >
                    )
                {
                    return changeNodeKinetics(
                        program,
                        mutation.nodeId,
                        mutation.change
                    );
                }

                else if constexpr (
                    std::is_same_v<
                    MutationType,
                    InteractionParameterMutationEvent
                    >
                    )
                {
                    const std::size_t index =
                        findInteractionIndex(
                            program,
                            mutation.sourceNodeId,
                            mutation.targetNodeId
                        );

                    return changeInteractionParameters(
                        program,
                        index,
                        mutation.change
                    );
                }

                else if constexpr (
                    std::is_same_v<
                    MutationType,
                    InteractionGainMutationEvent
                    >
                    )
                {
                    return addRegulatoryInteraction(
                        program,
                        mutation.interaction
                    );
                }

                else if constexpr (
                    std::is_same_v<
                    MutationType,
                    InteractionLossMutationEvent
                    >
                    )
                {
                    const std::size_t index =
                        findInteractionIndex(
                            program,
                            mutation.sourceNodeId,
                            mutation.targetNodeId
                        );

                    return removeRegulatoryInteraction(
                        program,
                        index
                    );
                }

                else if constexpr (
                    std::is_same_v<
                    MutationType,
                    RegulatoryUnitDuplicationMutationEvent
                    >
                    )
                {
                    return duplicateRegulatoryUnit(
                        program,
                        mutation.sourceNodeId,
                        mutation.newNodeId
                    );
                }

                else if constexpr (
                    std::is_same_v<
                    MutationType,
                    RegulatoryNodeLossMutationEvent
                    >
                    )
                {
                    return removeRegulatoryNode(
                        program,
                        mutation.nodeId
                    );
                }

                else
                {
                    static_assert(
                        !sizeof(MutationType),
                        "Unsupported regulatory mutation event."
                        );
                }
            },
            event
        );
    }

    RegulatoryProgram applyRegulatoryMutationEvents(
        const RegulatoryProgram& program,
        const std::vector<RegulatoryMutationEvent>& events
    )
    {
        RegulatoryProgram result =
            program;

        for (const RegulatoryMutationEvent& event :
            events)
        {
            result =
                applyRegulatoryMutationEvent(
                    result,
                    event
                );
        }

        return result;
    }

} // namespace ae