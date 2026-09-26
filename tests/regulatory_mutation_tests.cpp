#include <cmath>
#include <iostream>
#include <stdexcept>

#include "alien_evolution/genetics/RegulatoryMutation.hpp"
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
        const std::uint64_t sourceNodeId,
        const std::uint64_t targetNodeId
    )
    {
        for (const auto& interaction :
            program.interactions())
        {
            if (
                interaction.sourceNodeId
                == sourceNodeId
                && interaction.targetNodeId
                == targetNodeId
                )
            {
                return true;
            }
        }

        return false;
    }

    template <typename Function>
    void requireThrows(
        Function function,
        const char* message
    )
    {
        bool threw = false;

        try
        {
            function();
        }
        catch (const std::exception&)
        {
            threw = true;
        }

        require(
            threw,
            message
        );
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
                    2.0,
                    0.5
                }
            },
        {
            {
                1,
                2,
                2.0,
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
        // Node kinetic mutation changes only offspring.
        // --------------------------------------------------------

        const ae::RegulatoryProgram nodeMutant =
            ae::changeNodeKinetics(
                original,
                1,
                {
                    2.0,
                    0.5
                }
            );

        require(
            nearlyEqual(
                nodeMutant.node(1)
                .basalProductionRate,
                2.0
            ),
            "Node production mutation failed."
        );

        require(
            nearlyEqual(
                nodeMutant.node(1)
                .degradationRate,
                0.5
            ),
            "Node degradation mutation failed."
        );

        require(
            nearlyEqual(
                original.node(1)
                .basalProductionRate,
                1.0
            ),
            "Parent regulatory program was modified."
        );

        // --------------------------------------------------------
        // Test 2:
        // Interaction parameters evolve quantitatively.
        // --------------------------------------------------------

        const ae::RegulatoryProgram interactionMutant =
            ae::changeInteractionParameters(
                original,
                0,
                {
                    3.0,
                    2.0,
                    0.5
                }
            );

        const auto& changedInteraction =
            interactionMutant.interactions()[0];

        require(
            nearlyEqual(
                changedInteraction.foldChange,
                6.0
            ),
            "Fold-change mutation failed."
        );

        require(
            nearlyEqual(
                changedInteraction.halfSaturation,
                1.0
            ),
            "Half-saturation mutation failed."
        );

        require(
            nearlyEqual(
                changedInteraction.cooperativity,
                1.0
            ),
            "Cooperativity mutation failed."
        );

        // --------------------------------------------------------
        // Test 3:
        // Regulatory linkage gain.
        // --------------------------------------------------------

        const ae::RegulatoryProgram added =
            ae::addRegulatoryInteraction(
                original,
                {
                    2,
                    1,
                    0.2,
                    1.0,
                    2.0
                }
            );

        require(
            added.interactionCount() == 2,
            "Regulatory interaction gain failed."
        );

        require(
            original.interactionCount() == 1,
            "Interaction gain modified parent program."
        );

        // --------------------------------------------------------
        // Test 4:
        // Regulatory linkage loss.
        // --------------------------------------------------------

        const ae::RegulatoryProgram removed =
            ae::removeRegulatoryInteraction(
                original,
                0
            );

        require(
            removed.interactionCount() == 0,
            "Regulatory interaction loss failed."
        );

        // --------------------------------------------------------
        // Test 5:
        // Duplicate an ordinary regulatory node.
        //
        // Original:
        //
        // 1 -> 2
        //
        // Duplicate node 1 as node 3:
        //
        // 1 -> 2
        // 3 -> 2
        // --------------------------------------------------------

        const ae::RegulatoryProgram duplicated =
            ae::duplicateRegulatoryUnit(
                original,
                1,
                3
            );

        require(
            duplicated.nodeCount() == 3,
            "Regulatory duplication failed to create node."
        );

        require(
            duplicated.containsNode(3),
            "Duplicated node was not found."
        );

        require(
            nearlyEqual(
                duplicated.node(3)
                .basalProductionRate,
                original.node(1)
                .basalProductionRate
            ),
            "Duplicate failed to inherit node kinetics."
        );

        require(
            hasInteraction(
                duplicated,
                3,
                2
            ),
            "Duplicate failed to inherit outgoing regulatory interaction."
        );

        require(
            original.nodeCount() == 2,
            "Duplication modified parent program."
        );

        // --------------------------------------------------------
        // Test 6:
        // Incoming regulation is inherited.
        //
        // Original:
        //
        // 1 -> 2
        //
        // Duplicate node 2 as node 3:
        //
        // 1 -> 2
        // 1 -> 3
        // --------------------------------------------------------

        const ae::RegulatoryProgram duplicatedTarget =
            ae::duplicateRegulatoryUnit(
                original,
                2,
                3
            );

        require(
            hasInteraction(
                duplicatedTarget,
                1,
                3
            ),
            "Duplicate failed to inherit incoming regulatory interaction."
        );

        // --------------------------------------------------------
        // Test 7:
        // Self-regulatory unit duplication preserves corresponding
        // regulatory context.
        // --------------------------------------------------------

        const ae::RegulatoryProgram selfProgram(
            {
                {
                    10,
                    0.1,
                    1.0,
                    1.0
                }
            },
            {
                {
                    10,
                    10,
                    5.0,
                    0.5,
                    2.0
                }
            }
        );

        const ae::RegulatoryProgram selfDuplicate =
            ae::duplicateRegulatoryUnit(
                selfProgram,
                10,
                11
            );

        require(
            hasInteraction(
                selfDuplicate,
                10,
                10
            ),
            "Original self-regulation disappeared."
        );

        require(
            hasInteraction(
                selfDuplicate,
                10,
                11
            ),
            "Duplicated unit failed to inherit incoming self-regulation."
        );

        require(
            hasInteraction(
                selfDuplicate,
                11,
                10
            ),
            "Duplicated unit failed to inherit outgoing self-regulation."
        );

        require(
            hasInteraction(
                selfDuplicate,
                11,
                11
            ),
            "Duplicate failed to inherit self-regulatory relationship."
        );

        // --------------------------------------------------------
        // Test 8:
        // Regulatory node deletion removes all incident edges.
        // --------------------------------------------------------

        const ae::RegulatoryProgram deletionProgram(
            {
                {1, 0.0, 1.0, 1.0},
                {2, 0.0, 1.0, 1.0},
                {3, 0.0, 1.0, 1.0}
            },
            {
                {1, 2, 2.0, 0.5, 2.0},
                {2, 3, 2.0, 0.5, 2.0},
                {3, 1, 2.0, 0.5, 2.0}
            }
        );

        const ae::RegulatoryProgram deleted =
            ae::removeRegulatoryNode(
                deletionProgram,
                2
            );

        require(
            deleted.nodeCount() == 2,
            "Regulatory node deletion failed."
        );

        require(
            !deleted.containsNode(2),
            "Deleted regulatory node still exists."
        );

        require(
            deleted.interactionCount() == 1,
            "Incident interactions were not removed with node."
        );

        require(
            hasInteraction(
                deleted,
                3,
                1
            ),
            "Unrelated regulatory interaction was incorrectly removed."
        );

        // --------------------------------------------------------
        // Test 9:
        // Existing IDs cannot be reused for duplication.
        // --------------------------------------------------------

        requireThrows(
            [&original]
            {
                const auto mutant =
                    ae::duplicateRegulatoryUnit(
                        original,
                        1,
                        2
                    );
            },
            "Duplicate operation accepted an existing node ID."
        );

        // --------------------------------------------------------
        // Test 10:
        // Final regulatory node cannot be deleted.
        // --------------------------------------------------------

        requireThrows(
            []
            {
                const ae::RegulatoryProgram oneNode(
                    {
                        {
                            1,
                            0.0,
                            1.0,
                            1.0
                        }
                    },
                    {}
                );

                const auto deleted =
                    ae::removeRegulatoryNode(
                        oneNode,
                        1
                    );
            },
            "Final regulatory node was allowed to be deleted."
        );

        // --------------------------------------------------------
        // Test 11:
        // Invalid node operations are rejected.
        // --------------------------------------------------------

        requireThrows(
            [&original]
            {
                const auto mutant =
                    ae::duplicateRegulatoryUnit(
                        original,
                        999,
                        3
                    );
            },
            "Nonexistent regulatory node was duplicated."
        );

        requireThrows(
            [&original]
            {
                const auto mutant =
                    ae::removeRegulatoryNode(
                        original,
                        999
                    );
            },
            "Nonexistent regulatory node was deleted."
        );

        std::cout
            << "All regulatory mutation tests passed.\n";

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