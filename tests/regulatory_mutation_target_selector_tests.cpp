#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/genetics/RegulatoryMutationTargetSelector.hpp"
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
                interaction.sourceNodeId == sourceNodeId
                && interaction.targetNodeId == targetNodeId
                )
            {
                return true;
            }
        }

        return false;
    }

} // namespace

int main()
{
    try
    {
        const ae::RegulatoryProgram sparseProgram(
            {
                {1, 0.0, 1.0, 1.0},
                {2, 0.0, 1.0, 1.0},
                {3, 0.0, 1.0, 1.0},
                {4, 0.0, 1.0, 1.0}
            },
            {
                {1, 2, 2.0, 0.5, 2.0},
                {2, 3, 0.2, 0.5, 2.0},
                {3, 4, 2.0, 0.5, 2.0},
                {4, 1, 0.2, 0.5, 2.0}
            }
        );

        // --------------------------------------------------------
        // Test 1:
        // Identical seeds produce identical targets.
        // --------------------------------------------------------

        ae::Random randomA(123456);
        ae::Random randomB(123456);

        for (std::size_t i = 0;
            i < 1000;
            ++i)
        {
            require(
                ae::selectUniformNodeKineticTarget(
                    sparseProgram,
                    randomA
                )
                ==
                ae::selectUniformNodeKineticTarget(
                    sparseProgram,
                    randomB
                ),
                "Identical seeds produced different node targets."
            );

            const auto edgeA =
                ae::selectUniformInteractionGainTarget(
                    sparseProgram,
                    randomA
                );

            const auto edgeB =
                ae::selectUniformInteractionGainTarget(
                    sparseProgram,
                    randomB
                );

            require(
                edgeA.sourceNodeId
                == edgeB.sourceNodeId
                && edgeA.targetNodeId
                == edgeB.targetNodeId,
                "Identical seeds produced different interaction targets."
            );
        }

        // --------------------------------------------------------
        // Test 2:
        // Existing-edge selectors always choose existing edges.
        // --------------------------------------------------------

        ae::Random existingRandom(2222);

        for (std::size_t i = 0;
            i < 5000;
            ++i)
        {
            const auto parameterTarget =
                ae::selectUniformInteractionParameterTarget(
                    sparseProgram,
                    existingRandom
                );

            require(
                hasInteraction(
                    sparseProgram,
                    parameterTarget.sourceNodeId,
                    parameterTarget.targetNodeId
                ),
                "Interaction parameter selector chose absent edge."
            );

            const auto lossTarget =
                ae::selectUniformInteractionLossTarget(
                    sparseProgram,
                    existingRandom
                );

            require(
                hasInteraction(
                    sparseProgram,
                    lossTarget.sourceNodeId,
                    lossTarget.targetNodeId
                ),
                "Interaction loss selector chose absent edge."
            );
        }

        // --------------------------------------------------------
        // Test 3:
        // Sparse interaction-gain path always returns absent edge.
        // --------------------------------------------------------

        ae::Random sparseGainRandom(3333);

        for (std::size_t i = 0;
            i < 5000;
            ++i)
        {
            const auto target =
                ae::selectUniformInteractionGainTarget(
                    sparseProgram,
                    sparseGainRandom
                );

            require(
                !hasInteraction(
                    sparseProgram,
                    target.sourceNodeId,
                    target.targetNodeId
                ),
                "Sparse gain selector chose existing edge."
            );
        }

        // --------------------------------------------------------
        // Test 4:
        // Dense-network path correctly finds the only missing edge.
        //
        // Possible:
        //
        // 1->1, 1->2, 2->1, 2->2
        //
        // Only 2->2 is absent.
        // --------------------------------------------------------

        const ae::RegulatoryProgram denseProgram(
            {
                {1, 0.0, 1.0, 1.0},
                {2, 0.0, 1.0, 1.0}
            },
            {
                {1, 1, 2.0, 0.5, 2.0},
                {1, 2, 2.0, 0.5, 2.0},
                {2, 1, 2.0, 0.5, 2.0}
            }
        );

        ae::Random denseRandom(4444);

        for (std::size_t i = 0;
            i < 100;
            ++i)
        {
            const auto target =
                ae::selectUniformInteractionGainTarget(
                    denseProgram,
                    denseRandom
                );

            require(
                target.sourceNodeId == 2
                && target.targetNodeId == 2,
                "Dense gain selector failed to identify only absent edge."
            );
        }

        // --------------------------------------------------------
        // Test 5:
        // Uniform node null model is approximately uniform.
        // --------------------------------------------------------

        constexpr std::size_t sampleCount =
            40000;

        std::array<std::size_t, 4> counts{
            0,
            0,
            0,
            0
        };

        ae::Random uniformRandom(5555);

        for (std::size_t i = 0;
            i < sampleCount;
            ++i)
        {
            const std::uint64_t nodeId =
                ae::selectUniformNodeKineticTarget(
                    sparseProgram,
                    uniformRandom
                );

            require(
                nodeId >= 1
                && nodeId <= 4,
                "Node selector produced invalid ID."
            );

            ++counts[
                static_cast<std::size_t>(
                    nodeId - 1
                    )
            ];
        }

        const double expected =
            static_cast<double>(
                sampleCount
                )
            / 4.0;

        for (const std::size_t count :
        counts)
        {
            const double difference =
                std::abs(
                    static_cast<double>(count)
                    - expected
                );

            require(
                difference < 500.0,
                "Uniform node selector failed statistical uniformity check."
            );
        }

        // --------------------------------------------------------
        // Test 6:
        // A fully connected network has no gain target.
        // --------------------------------------------------------

        const ae::RegulatoryProgram fullProgram(
            {
                {1, 0.0, 1.0, 1.0}
            },
            {
                {1, 1, 2.0, 0.5, 2.0}
            }
        );

        requireThrows(
            [&fullProgram]
            {
                ae::Random random(1);

                const auto target =
                    ae::selectUniformInteractionGainTarget(
                        fullProgram,
                        random
                    );
            },
            "Fully connected network incorrectly provided gain target."
        );

        // --------------------------------------------------------
        // Test 7:
        // The final node cannot be selected for loss.
        // --------------------------------------------------------

        requireThrows(
            [&fullProgram]
            {
                ae::Random random(1);

                const auto target =
                    ae::selectUniformNodeLossTarget(
                        fullProgram,
                        random
                    );
            },
            "Final regulatory node was selected for deletion."
        );

        std::cout
            << "All regulatory mutation target selector tests passed.\n";

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