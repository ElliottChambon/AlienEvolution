#include <cmath>
#include <iostream>
#include <limits>
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
            throw std::runtime_error(
                message
            );
        }
    }


    bool nearlyEqual(
        const double a,
        const double b,
        const double tolerance = 1.0e-3
    )
    {
        return
            std::abs(
                a - b
            )
            <= tolerance;
    }

    template <typename Function>
    void requireInvalid(Function function)
    {
        try
        {
            function();
        }
        catch (const std::invalid_argument&)
        {
            return;
        }
        throw std::runtime_error("Invalid external-input integration arguments accepted.");
    }

    void testExternalInputs()
    {
        using Dynamics = ae::RegulatoryDynamics;
        const ae::RegulatoryProgram program({{10, 0.0, 2.0, 0.5}}, {});
        const ae::RegulatoryState initial{1.0};
        const ae::RegulatoryInputInterface empty(program, {});
        const ae::RegulatoryInputInterface activation(program, {{100, 10, 5.0, 2.0, 2.0}});
        const ae::RegulatoryInputInterface repression(program, {{100, 10, 0.25, 2.0, 2.0}});
        const std::vector<ae::ExternalSignalValue> signals{{100, 2.0}};
        const std::vector<ae::ExternalSignalValue> zero{{100, 0.0}};

        require(Dynamics::derivatives(program, initial)
            == Dynamics::derivatives(program, initial, empty, {}), "Empty input changed derivatives.");
        require(Dynamics::stepRK4(program, initial, 0.5)
            == Dynamics::stepRK4(program, initial, 0.5, empty, {}), "Empty input changed RK4.");
        require(Dynamics::simulate(program, initial, 1.25, 0.5)
            == Dynamics::simulate(program, initial, 1.25, 0.5, empty, {}), "Empty input changed simulation.");
        require(Dynamics::derivatives(program, initial)
            == Dynamics::derivatives(program, initial, activation, zero), "Zero signal changed derivatives.");
        require(Dynamics::stepRK4(program, initial, 0.5)
            == Dynamics::stepRK4(program, initial, 0.5, activation, zero), "Zero signal changed RK4.");
        require(Dynamics::simulate(program, initial, 1.25, 0.5)
            == Dynamics::simulate(program, initial, 1.25, 0.5, activation, zero), "Zero signal changed simulation.");

        const auto baseline = Dynamics::simulate(program, initial, 3.0, 0.01);
        const auto activated = Dynamics::simulate(program, initial, 3.0, 0.01, activation, signals);
        const auto repressed = Dynamics::simulate(program, initial, 3.0, 0.01, repression, signals);
        require(activated[0] > baseline[0] && repressed[0] < baseline[0],
            "External activation/repression failed.");
        // Exact solution of dx/dt = 6 - 0.5x with x(0)=1.
        require(nearlyEqual(activated[0], 12.0 - 11.0 * std::exp(-1.5), 1e-9),
            "Constant-input simulation disagrees with analytical solution.");
        require(activated.size() == initial.size(), "External signal entered regulatory state.");
        require(activated == Dynamics::simulate(program, initial, 3.0, 0.01, activation, signals),
            "External-input simulation is not deterministic.");

        // For dx/dt = 6 - 0.5x, x(0)=1 and h=1, RK4 yields 5.3255208333.
        // This oracle detects missing modulation at any derivative stage,
        // multiplying the full derivative, or scaling the completed step.
        const auto stageResult = Dynamics::stepRK4(program, initial, 1.0, activation, signals);
        require(nearlyEqual(stageResult[0], 5.325520833333333, 1e-12),
            "External production modulation was not applied correctly at every RK4 stage.");

        const ae::RegulatoryProgram intrinsic(
            {{20, 1.0, 1.0, 1.0}, {10, 1.0, 2.0, 0.5}},
            {{20, 10, 5.0, 1.0, 2.0}}
        );
        const ae::RegulatoryInputInterface combined(intrinsic, {{100, 10, 5.0, 2.0, 2.0}});
        const auto derivative = Dynamics::derivatives(intrinsic, {1.0, 1.0}, combined, signals);
        require(nearlyEqual(derivative[1], 17.5, 1e-12) && derivative[0] == 0.0,
            "Intrinsic/external factors did not multiply production or changed untargeted node.");
        // A constant external factor is equivalent to scaling basal production,
        // even when intrinsic self-regulation changes between RK4 stages.
        const ae::RegulatoryProgram feedback({{10, 0.4, 0.2, 1.0}}, {{10, 10, 5.0, 0.5, 2.0}});
        const ae::RegulatoryProgram scaled({{10, 0.4, 0.6, 1.0}}, {{10, 10, 5.0, 0.5, 2.0}});
        const auto feedbackStep = Dynamics::stepRK4(feedback, {0.4}, 0.5, activation, signals);
        require(nearlyEqual(feedbackStep[0], Dynamics::stepRK4(scaled, {0.4}, 0.5)[0], 1e-12),
            "External modulation failed with changing intrinsic feedback at RK4 stages.");

        const auto partial = Dynamics::simulate(program, initial, 1.25, 0.5, activation, signals);
        auto manual = Dynamics::stepRK4(program, initial, 0.5, activation, signals);
        manual = Dynamics::stepRK4(program, manual, 0.5, activation, signals);
        manual = Dynamics::stepRK4(program, manual, 0.25, activation, signals);
        require(partial == manual, "Constant-input simulation mishandled final partial step.");
        require(Dynamics::simulate(program, initial, 0.0, 0.5, activation, signals) == initial,
            "Zero-duration simulation changed state.");

        const ae::RegulatoryProgram other({{999, 0.0, 1.0, 1.0}}, {});
        // Check interface errors propagate through every entry point, including
        // a zero-duration simulation where no derivative would be evaluated.
        for (int api = 0; api < 3; ++api)
        {
            const auto evaluate = [&](const ae::RegulatoryProgram& p,
                const std::vector<ae::ExternalSignalValue>& values) {
                if (api == 0) (void)Dynamics::derivatives(p, initial, activation, values);
                if (api == 1) (void)Dynamics::stepRK4(p, initial, 0.1, activation, values);
                if (api == 2) (void)Dynamics::simulate(p, initial, 0.0, 0.1, activation, values);
            };
            requireInvalid([&] { evaluate(program, {}); });
            requireInvalid([&] { evaluate(program, {{100, 1.0}, {100, 2.0}}); });
            requireInvalid([&] { evaluate(other, signals); });
            for (double invalid : {-1.0, std::numeric_limits<double>::infinity(),
                std::numeric_limits<double>::quiet_NaN()})
            {
                requireInvalid([&] { evaluate(program, {{100, invalid}}); });
            }
        }
        requireInvalid([&] { (void)Dynamics::derivatives(program, {}, activation, signals); });
        requireInvalid([&] { (void)Dynamics::stepRK4(program, initial, 0.0, activation, signals); });
        requireInvalid([&] { (void)Dynamics::simulate(program, initial, -1.0, 0.1, activation, signals); });
        requireInvalid([&] { (void)Dynamics::simulate(program, initial, 1.0, 0.0, activation, signals); });
    }

} // namespace


int main()
{
    try
    {
        testExternalInputs();
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
        // Unregulated node approaches analytical equilibrium.
        //
        // dx/dt = 2 - 0.5x
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
            activated[1]
            > repressed[1],
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
            deterministicA
            == deterministicB,
            "Identical regulatory simulations produced different states."
        );


        // --------------------------------------------------------
        // Test 7:
        // External clamping must prevent intrinsic kinetics of a
        // clamped node from leaking through intermediate RK4 stages.
        // --------------------------------------------------------

        const ae::RegulatoryProgram slowInputProgram(
            {
                {
                    1,
                    0.0,
                    0.0,
                    1.0
                },
                {
                    2,
                    0.0,
                    0.2,
                    1.0
                }
            },
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


        const ae::RegulatoryProgram fastInputProgram(
            {
                {
                    1,
                    0.0,
                    10.0,
                    1.0
                },
                {
                    2,
                    0.0,
                    0.2,
                    1.0
                }
            },
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


        const std::vector<ae::RegulatoryStateClamp> clamps{
            {
                0,
                0.5
            }
        };


        ae::RegulatoryState clampedSlow{
            0.5,
            0.0
        };

        ae::RegulatoryState clampedFast{
            0.5,
            0.0
        };


        for (
            int step = 0;
            step < 20;
            ++step
            )
        {
            clampedSlow =
                ae::RegulatoryDynamics::stepRK4(
                    slowInputProgram,
                    clampedSlow,
                    0.1,
                    clamps
                );

            clampedFast =
                ae::RegulatoryDynamics::stepRK4(
                    fastInputProgram,
                    clampedFast,
                    0.1,
                    clamps
                );
        }


        require(
            clampedSlow[0] == 0.5
            && clampedFast[0] == 0.5,
            "Externally clamped node did not remain exactly fixed."
        );


        require(
            nearlyEqual(
                clampedSlow[1],
                clampedFast[1],
                1.0e-12
            ),
            "Intrinsic kinetics of clamped node leaked into downstream RK4 dynamics."
        );


        // Confirm the regression test would detect a leak:
        // without the clamp, the two input-kinetic models should
        // affect the downstream node differently.
        const ae::RegulatoryState unclampedSlow =
            ae::RegulatoryDynamics::simulate(
                slowInputProgram,
                { 0.5, 0.0 },
                2.0,
                0.1
            );

        const ae::RegulatoryState unclampedFast =
            ae::RegulatoryDynamics::simulate(
                fastInputProgram,
                { 0.5, 0.0 },
                2.0,
                0.1
            );

        require(
            std::abs(
                unclampedSlow[1]
                - unclampedFast[1]
            )
            > 1.0e-3,
            "RK4 clamp regression test setup is not sensitive to input kinetics."
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
