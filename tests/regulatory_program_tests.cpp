#include <iostream>
#include <stdexcept>
#include <vector>

#include "alien_evolution/genetics/RegulatoryInteraction.hpp"
#include "alien_evolution/genetics/RegulatoryNode.hpp"
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

} // namespace

int main()
{
    try
    {
        // --------------------------------------------------------
        // Test 1:
        // Valid regulatory program.
        // --------------------------------------------------------

        std::vector<ae::RegulatoryNode> nodes{
            {
                1,
                0.1,
                1.0,
                0.5
            },
            {
                2,
                0.0,
                1.5,
                0.8
            }
        };

        std::vector<ae::RegulatoryInteraction> interactions{
            {
                1,
                2,
                1.0,
                0.5,
                2.0
            },
            {
                2,
                1,
                -1.0,
                0.5,
                2.0
            }
        };

        const ae::RegulatoryProgram program(
            nodes,
            interactions
        );

        require(
            program.nodeCount() == 2,
            "Incorrect regulatory node count."
        );

        require(
            program.interactionCount() == 2,
            "Incorrect regulatory interaction count."
        );

        require(
            program.containsNode(1),
            "Expected regulatory node was not found."
        );

        require(
            program.node(2).id == 2,
            "Incorrect regulatory node returned."
        );

        // --------------------------------------------------------
        // Test 2:
        // Duplicate node IDs are invalid.
        // --------------------------------------------------------

        requireThrows(
            []
            {
                const std::vector<ae::RegulatoryNode> badNodes{
                    {1, 0.0, 1.0, 1.0},
                    {1, 0.0, 1.0, 1.0}
                };

                const ae::RegulatoryProgram badProgram(
                    badNodes,
                    {}
                );
            },
            "Duplicate regulatory node IDs were accepted."
        );

        // --------------------------------------------------------
        // Test 3:
        // Interaction endpoints must exist.
        // --------------------------------------------------------

        requireThrows(
            []
            {
                const std::vector<ae::RegulatoryNode> oneNode{
                    {1, 0.0, 1.0, 1.0}
                };

                const std::vector<ae::RegulatoryInteraction> badInteractions{
                    {
                        1,
                        999,
                        1.0,
                        0.5,
                        1.0
                    }
                };

                const ae::RegulatoryProgram badProgram(
                    oneNode,
                    badInteractions
                );
            },
            "Interaction with missing target node was accepted."
        );

        // --------------------------------------------------------
        // Test 4:
        // Negative degradation is invalid.
        // --------------------------------------------------------

        requireThrows(
            []
            {
                const std::vector<ae::RegulatoryNode> badNodes{
                    {
                        1,
                        0.0,
                        1.0,
                        -1.0
                    }
                };

                const ae::RegulatoryProgram badProgram(
                    badNodes,
                    {}
                );
            },
            "Negative degradation rate was accepted."
        );

        // --------------------------------------------------------
        // Test 5:
        // Invalid Hill parameters are rejected.
        // --------------------------------------------------------

        requireThrows(
            []
            {
                const std::vector<ae::RegulatoryNode> validNodes{
                    {1, 0.0, 1.0, 1.0}
                };

                const std::vector<ae::RegulatoryInteraction> badInteractions{
                    {
                        1,
                        1,
                        1.0,
                        0.0,
                        1.0
                    }
                };

                const ae::RegulatoryProgram badProgram(
                    validNodes,
                    badInteractions
                );
            },
            "Non-positive half-saturation was accepted."
        );

        requireThrows(
            []
            {
                const std::vector<ae::RegulatoryNode> validNodes{
                    {1, 0.0, 1.0, 1.0}
                };

                const std::vector<ae::RegulatoryInteraction> badInteractions{
                    {
                        1,
                        1,
                        1.0,
                        0.5,
                        0.0
                    }
                };

                const ae::RegulatoryProgram badProgram(
                    validNodes,
                    badInteractions
                );
            },
            "Non-positive cooperativity was accepted."
        );

        std::cout
            << "All regulatory program tests passed.\n";

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