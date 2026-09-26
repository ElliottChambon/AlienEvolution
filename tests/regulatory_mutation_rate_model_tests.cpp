#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "alien_evolution/genetics/RegulatoryMutationRateModel.hpp"
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

} // namespace

int main()
{
    try
    {
        // --------------------------------------------------------
        // Test 1:
        // Verify mutational opportunity counting.
        //
        // Three nodes create:
        //
        // 3 x 3 = 9
        //
        // possible directed relationships including self-loops.
        //
        // Two relationships already exist, so seven possible
        // regulatory gains remain.
        // --------------------------------------------------------

        const ae::RegulatoryProgram program(
            {
                {1, 0.0, 1.0, 1.0},
                {2, 0.0, 1.0, 1.0},
                {3, 0.0, 1.0, 1.0}
            },
            {
                {1, 2, 2.0, 0.5, 2.0},
                {2, 3, 0.2, 0.5, 2.0}
            }
        );

        const ae::RegulatoryMutationRateModel rates{
            0.10,
            0.20,
            0.30,
            0.40,
            0.50,
            0.60
        };

        const ae::RegulatoryMutationHazards hazards =
            ae::computeRegulatoryMutationHazards(
                program,
                rates
            );

        require(
            hazards.eligibleNodeKineticTargets == 3,
            "Incorrect number of node kinetic targets."
        );

        require(
            hazards.eligibleInteractionParameterTargets == 2,
            "Incorrect number of interaction parameter targets."
        );

        require(
            hazards.eligibleInteractionGainTargets == 7,
            "Incorrect number of interaction gain targets."
        );

        require(
            hazards.eligibleInteractionLossTargets == 2,
            "Incorrect number of interaction loss targets."
        );

        require(
            hazards.eligibleDuplicationTargets == 3,
            "Incorrect number of duplication targets."
        );

        require(
            hazards.eligibleNodeLossTargets == 3,
            "Incorrect number of node-loss targets."
        );


        // --------------------------------------------------------
        // Test 2:
        // Expected event counts scale with eligible target count.
        // --------------------------------------------------------

        require(
            nearlyEqual(
                hazards.expectedNodeKineticEvents,
                0.30
            ),
            "Incorrect node kinetic mutation hazard."
        );

        require(
            nearlyEqual(
                hazards.expectedInteractionParameterEvents,
                0.40
            ),
            "Incorrect interaction parameter mutation hazard."
        );

        require(
            nearlyEqual(
                hazards.expectedInteractionGainEvents,
                2.10
            ),
            "Incorrect interaction gain hazard."
        );

        require(
            nearlyEqual(
                hazards.expectedInteractionLossEvents,
                0.80
            ),
            "Incorrect interaction loss hazard."
        );

        require(
            nearlyEqual(
                hazards.expectedDuplicationEvents,
                1.50
            ),
            "Incorrect duplication hazard."
        );

        require(
            nearlyEqual(
                hazards.expectedNodeLossEvents,
                1.80
            ),
            "Incorrect node-loss hazard."
        );

        require(
            nearlyEqual(
                hazards.totalExpectedEvents,
                6.90
            ),
            "Incorrect total mutation hazard."
        );


        // --------------------------------------------------------
        // Test 3:
        // A fully connected one-node program has no possible
        // interaction gain and cannot lose its final node.
        // --------------------------------------------------------

        const ae::RegulatoryProgram oneNodeProgram(
            {
                {1, 0.0, 1.0, 1.0}
            },
            {
                {1, 1, 2.0, 0.5, 2.0}
            }
        );

        const ae::RegulatoryMutationHazards oneNodeHazards =
            ae::computeRegulatoryMutationHazards(
                oneNodeProgram,
                rates
            );

        require(
            oneNodeHazards.eligibleInteractionGainTargets == 0,
            "Fully connected one-node network incorrectly permits interaction gain."
        );

        require(
            oneNodeHazards.eligibleNodeLossTargets == 0,
            "Final regulatory node incorrectly permits deletion."
        );

        require(
            nearlyEqual(
                oneNodeHazards.expectedInteractionGainEvents,
                0.0
            ),
            "Impossible interaction gain produced nonzero hazard."
        );

        require(
            nearlyEqual(
                oneNodeHazards.expectedNodeLossEvents,
                0.0
            ),
            "Impossible node loss produced nonzero hazard."
        );


        // --------------------------------------------------------
        // Test 4:
        // All-zero rate model generates zero expected mutations.
        // --------------------------------------------------------

        const ae::RegulatoryMutationRateModel zeroRates{};

        const ae::RegulatoryMutationHazards zeroHazards =
            ae::computeRegulatoryMutationHazards(
                program,
                zeroRates
            );

        require(
            nearlyEqual(
                zeroHazards.totalExpectedEvents,
                0.0
            ),
            "Zero mutation-rate model produced mutation hazard."
        );


        // --------------------------------------------------------
        // Test 5:
        // Invalid rates are rejected.
        // --------------------------------------------------------

        requireThrows(
            [&program]
            {
                ae::RegulatoryMutationRateModel badRates{};

                badRates.nodeKineticPerNode =
                    -0.1;

                const auto hazards =
                    ae::computeRegulatoryMutationHazards(
                        program,
                        badRates
                    );
            },
            "Negative mutation rate was accepted."
        );

        requireThrows(
            [&program]
            {
                ae::RegulatoryMutationRateModel badRates{};

                badRates.interactionGainPerAbsentPair =
                    std::numeric_limits<double>::infinity();

                const auto hazards =
                    ae::computeRegulatoryMutationHazards(
                        program,
                        badRates
                    );
            },
            "Infinite mutation rate was accepted."
        );


        std::cout
            << "All regulatory mutation rate model tests passed.\n";

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