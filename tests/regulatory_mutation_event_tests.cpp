#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "alien_evolution/genetics/RegulatoryMutationEvent.hpp"
#include "alien_evolution/genetics/RegulatoryProgram.hpp"

namespace
{

    void require(
        const bool condition,
        const char* message
    )
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    bool nearlyEqual(
        const double a,
        const double b,
        const double tolerance = 1.0e-12
    )
    {
        return
            std::abs(a - b)
            <= tolerance;
    }

    bool hasInteraction(
        const ae::RegulatoryProgram& program,
        const std::uint64_t source,
        const std::uint64_t target
    )
    {
        for (const auto& interaction :
            program.interactions())
        {
            if (
                interaction.sourceNodeId == source
                && interaction.targetNodeId == target
                )
            {
                return true;
            }
        }

        return false;
    }

    ae::RegulatoryProgram makeProgram()
    {
        return ae::RegulatoryProgram(
            {
                {
                    1,
                    0.0,
                    1.0,
                    1.0
                },
                {
                    2,
                    0.0,
                    1.0,
                    1.0
                },
                {
                    3,
                    0.0,
                    1.0,
                    1.0
                }
            },
        {
            {
                1,
                2,
                2.0,
                0.5,
                2.0
            },
            {
                2,
                3,
                0.2,
                0.5,
                2.0
            }
        }
        );
    }

} // namespace

int main()
{
    try
    {
        const ae::RegulatoryProgram original =
            makeProgram();

        // --------------------------------------------------------
        // Test 1:
        // Node kinetic mutation event.
        // --------------------------------------------------------

        const ae::RegulatoryMutationEvent nodeEvent =
            ae::NodeKineticMutationEvent{
                1,
                {
                    2.0,
                    0.5
                }
        };

        const ae::RegulatoryProgram nodeMutant =
            ae::applyRegulatoryMutationEvent(
                original,
                nodeEvent
            );

        require(
            nearlyEqual(
                nodeMutant.node(1)
                .basalProductionRate,
                2.0
            ),
            "Node kinetic event failed."
        );

        require(
            nearlyEqual(
                original.node(1)
                .basalProductionRate,
                1.0
            ),
            "Mutation event modified parent."
        );

        // --------------------------------------------------------
        // Test 2:
        // Interaction parameter event uses stable endpoint IDs.
        // --------------------------------------------------------

        const ae::RegulatoryMutationEvent parameterEvent =
            ae::InteractionParameterMutationEvent{
                1,
                2,
                {
                    3.0,
                    2.0,
                    0.5
                }
        };

        const ae::RegulatoryProgram parameterMutant =
            ae::applyRegulatoryMutationEvent(
                original,
                parameterEvent
            );

        require(
            nearlyEqual(
                parameterMutant.interactions()[0]
                .foldChange,
                6.0
            ),
            "Interaction parameter event failed."
        );

        // --------------------------------------------------------
        // Test 3:
        // Interaction gain event.
        // --------------------------------------------------------

        const ae::RegulatoryMutationEvent gainEvent =
            ae::InteractionGainMutationEvent{
                {
                    3,
                    1,
                    4.0,
                    0.7,
                    2.0
                }
        };

        const ae::RegulatoryProgram gained =
            ae::applyRegulatoryMutationEvent(
                original,
                gainEvent
            );

        require(
            hasInteraction(
                gained,
                3,
                1
            ),
            "Interaction gain event failed."
        );

        // --------------------------------------------------------
        // Test 4:
        // Interaction loss event.
        // --------------------------------------------------------

        const ae::RegulatoryMutationEvent lossEvent =
            ae::InteractionLossMutationEvent{
                1,
                2
        };

        const ae::RegulatoryProgram lost =
            ae::applyRegulatoryMutationEvent(
                original,
                lossEvent
            );

        require(
            !hasInteraction(
                lost,
                1,
                2
            ),
            "Interaction loss event failed."
        );

        // --------------------------------------------------------
        // Test 5:
        // Regulatory-unit duplication event.
        // --------------------------------------------------------

        const ae::RegulatoryMutationEvent duplicationEvent =
            ae::RegulatoryUnitDuplicationMutationEvent{
                1,
                4
        };

        const ae::RegulatoryProgram duplicated =
            ae::applyRegulatoryMutationEvent(
                original,
                duplicationEvent
            );

        require(
            duplicated.containsNode(4),
            "Duplication event failed."
        );

        require(
            hasInteraction(
                duplicated,
                4,
                2
            ),
            "Duplicated node failed to inherit regulatory context."
        );

        // --------------------------------------------------------
        // Test 6:
        // Regulatory-node loss event.
        // --------------------------------------------------------

        const ae::RegulatoryMutationEvent nodeLossEvent =
            ae::RegulatoryNodeLossMutationEvent{
                2
        };

        const ae::RegulatoryProgram nodeLost =
            ae::applyRegulatoryMutationEvent(
                original,
                nodeLossEvent
            );

        require(
            !nodeLost.containsNode(2),
            "Node-loss event failed."
        );

        require(
            nodeLost.interactionCount() == 0,
            "Node-loss event failed to remove incident interactions."
        );

        // --------------------------------------------------------
        // Test 7:
        // A sequence of mutation events is applied in order.
        //
        // Start:
        //
        // 1 -> 2 -> 3
        //
        // Duplicate 1 as 4:
        //
        // 1 -> 2 -> 3
        // 4 -> 2
        //
        // Then remove 1 -> 2:
        //
        // 4 -> 2 -> 3
        //
        // Then alter 4 -> 2.
        // --------------------------------------------------------

        const std::vector<ae::RegulatoryMutationEvent> events{
            ae::RegulatoryUnitDuplicationMutationEvent{
                1,
                4
            },

            ae::InteractionLossMutationEvent{
                1,
                2
            },

            ae::InteractionParameterMutationEvent{
                4,
                2,
                {
                    2.0,
                    1.0,
                    1.0
                }
            }
        };

        const ae::RegulatoryProgram sequenceResult =
            ae::applyRegulatoryMutationEvents(
                original,
                events
            );

        require(
            sequenceResult.containsNode(4),
            "Sequential duplication failed."
        );

        require(
            !hasInteraction(
                sequenceResult,
                1,
                2
            ),
            "Sequential interaction loss failed."
        );

        require(
            hasInteraction(
                sequenceResult,
                4,
                2
            ),
            "Sequential duplicate interaction disappeared."
        );

        bool foundMutatedEdge = false;

        for (const auto& interaction :
            sequenceResult.interactions())
        {
            if (
                interaction.sourceNodeId == 4
                && interaction.targetNodeId == 2
                )
            {
                foundMutatedEdge = true;

                require(
                    nearlyEqual(
                        interaction.foldChange,
                        4.0
                    ),
                    "Sequential interaction parameter mutation failed."
                );
            }
        }

        require(
            foundMutatedEdge,
            "Expected mutated interaction was not found."
        );

        // --------------------------------------------------------
        // Test 8:
        // Applying the same event sequence twice to identical
        // parents produces identical outcomes.
        // --------------------------------------------------------

        const ae::RegulatoryProgram deterministicA =
            ae::applyRegulatoryMutationEvents(
                original,
                events
            );

        const ae::RegulatoryProgram deterministicB =
            ae::applyRegulatoryMutationEvents(
                original,
                events
            );

        require(
            deterministicA.nodeCount()
            == deterministicB.nodeCount(),
            "Mutation event sequence changed node count nondeterministically."
        );

        require(
            deterministicA.interactionCount()
            == deterministicB.interactionCount(),
            "Mutation event sequence changed edge count nondeterministically."
        );

        for (std::size_t i = 0;
            i < deterministicA.nodeCount();
            ++i)
        {
            const auto& a =
                deterministicA.nodes()[i];

            const auto& b =
                deterministicB.nodes()[i];

            require(
                a.id == b.id
                && nearlyEqual(
                    a.basalProductionRate,
                    b.basalProductionRate
                )
                && nearlyEqual(
                    a.degradationRate,
                    b.degradationRate
                ),
                "Identical mutation event sequences produced different nodes."
            );
        }

        std::cout
            << "All regulatory mutation event tests passed.\n";

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr
            << "TEST FAILURE: "
            << error.what()
            << '\n';

        return 1;
    }
}