#include "alien_evolution/genetics/RegulatoryMutationRateModel.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace ae
{

    namespace
    {

        void validateRate(
            const double rate,
            const char* message
        )
        {
            if (
                !std::isfinite(rate)
                || rate < 0.0
                )
            {
                throw std::invalid_argument(
                    message
                );
            }
        }

        double expectedEvents(
            const double rate,
            const std::size_t targetCount
        )
        {
            const double result =
                rate
                * static_cast<double>(
                    targetCount
                    );

            if (!std::isfinite(result))
            {
                throw std::overflow_error(
                    "Mutation hazard became non-finite."
                );
            }

            return result;
        }

    } // namespace

    RegulatoryMutationHazards computeRegulatoryMutationHazards(
        const RegulatoryProgram& program,
        const RegulatoryMutationRateModel& rates
    )
    {
        validateRate(
            rates.nodeKineticPerNode,
            "Node kinetic mutation rate must be finite and nonnegative."
        );

        validateRate(
            rates.interactionParameterPerInteraction,
            "Interaction parameter mutation rate must be finite and nonnegative."
        );

        validateRate(
            rates.interactionGainPerAbsentPair,
            "Interaction gain rate must be finite and nonnegative."
        );

        validateRate(
            rates.interactionLossPerInteraction,
            "Interaction loss rate must be finite and nonnegative."
        );

        validateRate(
            rates.regulatoryUnitDuplicationPerNode,
            "Regulatory-unit duplication rate must be finite and nonnegative."
        );

        validateRate(
            rates.regulatoryNodeLossPerDeletableNode,
            "Regulatory-node loss rate must be finite and nonnegative."
        );

        const std::size_t nodeCount =
            program.nodeCount();

        const std::size_t interactionCount =
            program.interactionCount();

        if (
            nodeCount
                > std::numeric_limits<std::size_t>::max()
            / nodeCount
            )
        {
            throw std::overflow_error(
                "Regulatory network is too large to count possible interactions."
            );
        }

        // Directed regulatory relationships are allowed between
        // every ordered pair of nodes, including self-regulation.
        const std::size_t possibleInteractionCount =
            nodeCount * nodeCount;

        if (
            interactionCount
        > possibleInteractionCount
            )
        {
            throw std::logic_error(
                "Regulatory program contains more interactions than unique directed pairs."
            );
        }

        RegulatoryMutationHazards hazards{};

        hazards.eligibleNodeKineticTargets =
            nodeCount;

        hazards.eligibleInteractionParameterTargets =
            interactionCount;

        hazards.eligibleInteractionGainTargets =
            possibleInteractionCount
            - interactionCount;

        hazards.eligibleInteractionLossTargets =
            interactionCount;

        hazards.eligibleDuplicationTargets =
            nodeCount;

        // We preserve the invariant that a regulatory program
        // always contains at least one node.
        hazards.eligibleNodeLossTargets =
            nodeCount > 1
            ? nodeCount
            : 0;


        hazards.expectedNodeKineticEvents =
            expectedEvents(
                rates.nodeKineticPerNode,
                hazards.eligibleNodeKineticTargets
            );

        hazards.expectedInteractionParameterEvents =
            expectedEvents(
                rates.interactionParameterPerInteraction,
                hazards.eligibleInteractionParameterTargets
            );

        hazards.expectedInteractionGainEvents =
            expectedEvents(
                rates.interactionGainPerAbsentPair,
                hazards.eligibleInteractionGainTargets
            );

        hazards.expectedInteractionLossEvents =
            expectedEvents(
                rates.interactionLossPerInteraction,
                hazards.eligibleInteractionLossTargets
            );

        hazards.expectedDuplicationEvents =
            expectedEvents(
                rates.regulatoryUnitDuplicationPerNode,
                hazards.eligibleDuplicationTargets
            );

        hazards.expectedNodeLossEvents =
            expectedEvents(
                rates.regulatoryNodeLossPerDeletableNode,
                hazards.eligibleNodeLossTargets
            );


        hazards.totalExpectedEvents =
            hazards.expectedNodeKineticEvents
            + hazards.expectedInteractionParameterEvents
            + hazards.expectedInteractionGainEvents
            + hazards.expectedInteractionLossEvents
            + hazards.expectedDuplicationEvents
            + hazards.expectedNodeLossEvents;

        if (!std::isfinite(hazards.totalExpectedEvents))
        {
            throw std::overflow_error(
                "Total regulatory mutation hazard became non-finite."
            );
        }

        return hazards;
    }

} // namespace ae