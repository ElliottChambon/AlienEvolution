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
        // Node kinetic mutation changes only the offspring program.
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
        // Interaction parameters can evolve quantitatively.
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
        // New regulatory linkage can be gained.
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
        // Existing regulatory linkage can be lost.
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

        require(
            original.interactionCount() == 1,
            "Interaction loss modified parent program."
        );

        // --------------------------------------------------------
        // Test 5:
        // Duplicate directed interaction is rejected.
        // --------------------------------------------------------

        requireThrows(
            [&original]
            {
                const auto duplicate =
                    ae::addRegulatoryInteraction(
                        original,
                        {
                            1,
                            2,
                            4.0,
                            0.5,
                            1.0
                        }
                    );
            },
            "Duplicate regulatory interaction was accepted."
        );

        // --------------------------------------------------------
        // Test 6:
        // Invalid targets are rejected.
        // --------------------------------------------------------

        requireThrows(
            [&original]
            {
                const auto mutant =
                    ae::changeNodeKinetics(
                        original,
                        999,
                        {
                            2.0,
                            1.0
                        }
                    );
            },
            "Mutation of nonexistent node was accepted."
        );

        requireThrows(
            [&original]
            {
                const auto mutant =
                    ae::removeRegulatoryInteraction(
                        original,
                        999
                    );
            },
            "Removal of nonexistent interaction was accepted."
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