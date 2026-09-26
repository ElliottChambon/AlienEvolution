#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "alien_evolution/genetics/RegulatoryDynamics.hpp"
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

    bool nearlyEqual(
        const double a,
        const double b,
        const double tolerance = 1.0e-3
    )
    {
        return
            std::abs(a - b)
            <= tolerance;
    }

} // namespace

int main()
{
    try
    {
        // --------------------------------------------------------
        // Test 1:
        // Shifted Hill response has correct limiting behavior.
        // --------------------------------------------------------

        const double zeroActivation =
            ae::RegulatoryDynamics::shiftedHill(
                0.0,
                1.0,
                2.0,
                5.0
            );

        require(
            nearlyEqual(
                zeroActivation,
                1.0
            ),
            "Inactive regulator should produce baseline response."
        );

        const double halfActivation =
            ae::RegulatoryDynamics::shiftedHill(
                1.0,
                1.0,
                2.0,
                5.0
            );

        require(
            nearlyEqual(
                halfActivation,
                3.0
            ),
            "Shifted Hill response is incorrect at half-saturation."
        );

        const double halfRepression =
            ae::RegulatoryDynamics::shiftedHill(
                1.0,
                1.0,
                2.0,
                0.1
            );

        require(
            nearlyEqual(
                halfRepression,
                0.55
            ),
            "Repressive shifted Hill response is incorrect."
        );

        // --------------------------------------------------------
        // Test 2:
        // An unregulated node approaches production/degradation
        // equilibrium.
        //
        // dx/dt = 2 - 0.5x
        //
        // equilibrium x = 4
        // --------------------------------------------------------

        const ae::RegulatoryProgram unregulatedProgram(
            {
                {
                    1,
                    0.0,
                    2.0,
                    0.5
                }
            },
            {}
        );

        const ae::RegulatoryState unregulatedFinal =
            ae::RegulatoryDynamics::simulate(
                unregulatedProgram,
                { 0.0 },
                20.0,
                0.01
            );

        require(
            nearlyEqual(
                unregulatedFinal[0],
                4.0
            ),
            "Unregulated node failed to approach analytical equilibrium."
        );

        // --------------------------------------------------------
        // Test 3:
        // Activation increases target activity while repression
        // decreases it.
        // --------------------------------------------------------

        const std::vector<ae::RegulatoryNode> regulationNodes{
            {
                1,
                0.0,
                1.0,
                1.0
            },
            {
                2,
                0.0,
                0.2,
                1.0
            }
        };

        const ae::RegulatoryProgram activationProgram(
            regulationNodes,
            {
                {
                    1,
                    2,
                    5.0,
                    0.5,
                    2.0
                }
            }
        );

        const ae::RegulatoryProgram repressionProgram(
            regulationNodes,
            {
                {
                    1,
                    2,
                    0.1,
                    0.5,
                    2.0
                }
            }
        );

        const ae::RegulatoryState activated =
            ae::RegulatoryDynamics::simulate(
                activationProgram,
                { 0.0, 0.0 },
                50.0,
                0.01
            );

        const ae::RegulatoryState repressed =
            ae::RegulatoryDynamics::simulate(
                repressionProgram,
                { 0.0, 0.0 },
                50.0,
                0.01
            );

        require(
            activated[1] > 0.7,
            "Activation failed to increase target regulatory state."
        );

        require(
            repressed[1] < 0.1,
            "Repression failed to reduce target regulatory state."
        );

        require(
            activated[1] > repressed[1],
            "Activation did not exceed repression."
        );

        // --------------------------------------------------------
        // Test 4:
        // Positive feedback can produce two alternative stable
        // regulatory states.
        // --------------------------------------------------------

        const ae::RegulatoryProgram positiveFeedbackProgram(
            {
                {
                    1,
                    0.0,
                    0.1,
                    1.0
                }
            },
            {
                {
                    1,
                    1,
                    20.0,
                    0.5,
                    4.0
                }
            }
        );

        const ae::RegulatoryState lowState =
            ae::RegulatoryDynamics::simulate(
                positiveFeedbackProgram,
                { 0.1 },
                100.0,
                0.01
            );

        const ae::RegulatoryState highState =
            ae::RegulatoryDynamics::simulate(
                positiveFeedbackProgram,
                { 1.0 },
                100.0,
                0.01
            );

        require(
            lowState[0] < 0.2,
            "Positive-feedback low state was not maintained."
        );

        require(
            highState[0] > 1.5,
            "Positive-feedback high state was not maintained."
        );

        require(
            highState[0]
                > lowState[0] * 5.0,
            "Positive feedback failed to produce distinct stable states."
        );

        // --------------------------------------------------------
        // Test 5:
        // Mutual repression produces alternative dominant states.
        //
        // This is the topology used by classical genetic toggle
        // switches.
        // --------------------------------------------------------

        const ae::RegulatoryProgram toggleProgram(
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
                }
            },
            {
                {
                    1,
                    2,
                    0.01,
                    0.5,
                    4.0
                },
                {
                    2,
                    1,
                    0.01,
                    0.5,
                    4.0
                }
            }
        );

        const ae::RegulatoryState stateA =
            ae::RegulatoryDynamics::simulate(
                toggleProgram,
                { 1.0, 0.0 },
                50.0,
                0.01
            );

        require(
            stateA[0] > 0.8
            && stateA[1] < 0.2,
            "Mutual-repression state A was not stable."
        );

        const ae::RegulatoryState stateB =
            ae::RegulatoryDynamics::simulate(
                toggleProgram,
                { 0.0, 1.0 },
                50.0,
                0.01
            );

        require(
            stateB[1] > 0.8
            && stateB[0] < 0.2,
            "Mutual-repression state B was not stable."
        );

        // --------------------------------------------------------
        // Test 6:
        // Identical simulations are deterministic.
        // --------------------------------------------------------

        const ae::RegulatoryState deterministicA =
            ae::RegulatoryDynamics::simulate(
                toggleProgram,
                { 1.0, 0.0 },
                10.0,
                0.01
            );

        const ae::RegulatoryState deterministicB =
            ae::RegulatoryDynamics::simulate(
                toggleProgram,
                { 1.0, 0.0 },
                10.0,
                0.01
            );

        require(
            deterministicA == deterministicB,
            "Identical regulatory simulations produced different states."
        );

        std::cout
            << "All regulatory dynamics tests passed.\n";

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