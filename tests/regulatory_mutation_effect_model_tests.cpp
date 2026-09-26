#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/genetics/RegulatoryMutationEffectModel.hpp"

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
        const double tolerance
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
        // Default model has zero effect magnitude.
        // --------------------------------------------------------

        const ae::RegulatoryMutationEffectModel zeroModel{};

        ae::Random zeroRandom(12345);

        const ae::NodeKineticChange zeroNode =
            ae::sampleNodeKineticChange(
                zeroModel,
                zeroRandom
            );

        const ae::InteractionParameterChange zeroInteraction =
            ae::sampleInteractionParameterChange(
                zeroModel,
                zeroRandom
            );

        require(
            zeroNode.basalProductionMultiplier == 1.0
            && zeroNode.degradationMultiplier == 1.0,
            "Zero node-effect model produced a non-neutral change."
        );

        require(
            zeroInteraction.foldChangeMultiplier == 1.0
            && zeroInteraction.halfSaturationMultiplier == 1.0
            && zeroInteraction.cooperativityMultiplier == 1.0,
            "Zero interaction-effect model produced a non-neutral change."
        );


        // --------------------------------------------------------
        // Test 2:
        // Identical seeds produce identical quantitative effects.
        // --------------------------------------------------------

        const ae::RegulatoryMutationEffectModel deterministicModel{
            0.20,
            0.30,
            0.40,
            0.50,
            0.60
        };

        ae::Random randomA(987654321);
        ae::Random randomB(987654321);

        for (std::size_t i = 0;
            i < 1000;
            ++i)
        {
            const auto nodeA =
                ae::sampleNodeKineticChange(
                    deterministicModel,
                    randomA
                );

            const auto nodeB =
                ae::sampleNodeKineticChange(
                    deterministicModel,
                    randomB
                );

            require(
                nodeA.basalProductionMultiplier
                == nodeB.basalProductionMultiplier
                && nodeA.degradationMultiplier
                == nodeB.degradationMultiplier,
                "Identical seeds produced different node effects."
            );

            const auto interactionA =
                ae::sampleInteractionParameterChange(
                    deterministicModel,
                    randomA
                );

            const auto interactionB =
                ae::sampleInteractionParameterChange(
                    deterministicModel,
                    randomB
                );

            require(
                interactionA.foldChangeMultiplier
                == interactionB.foldChangeMultiplier
                && interactionA.halfSaturationMultiplier
                == interactionB.halfSaturationMultiplier
                && interactionA.cooperativityMultiplier
                == interactionB.cooperativityMultiplier,
                "Identical seeds produced different interaction effects."
            );
        }


        // --------------------------------------------------------
        // Test 3:
        // Multiplicative effects always remain positive.
        // --------------------------------------------------------

        const ae::RegulatoryMutationEffectModel positiveModel{
            1.0,
            1.0,
            1.0,
            1.0,
            1.0
        };

        ae::Random positiveRandom(3333);

        for (std::size_t i = 0;
            i < 10000;
            ++i)
        {
            const auto node =
                ae::sampleNodeKineticChange(
                    positiveModel,
                    positiveRandom
                );

            const auto interaction =
                ae::sampleInteractionParameterChange(
                    positiveModel,
                    positiveRandom
                );

            require(
                node.basalProductionMultiplier > 0.0
                && node.degradationMultiplier > 0.0
                && interaction.foldChangeMultiplier > 0.0
                && interaction.halfSaturationMultiplier > 0.0
                && interaction.cooperativityMultiplier > 0.0,
                "Log-space mutation produced nonpositive multiplier."
            );
        }


        // --------------------------------------------------------
        // Test 4:
        // Mean effect in LOG SPACE approaches zero.
        //
        // Important:
        // The arithmetic mean of exp(epsilon) is not 1.
        // The null model is symmetric in log space.
        // --------------------------------------------------------

        constexpr std::size_t sampleCount =
            30000;

        ae::RegulatoryMutationEffectModel statisticalModel{};

        statisticalModel.nodeBasalProductionLogStdDev =
            0.5;

        ae::Random statisticalRandom(4444);

        double sumLogEffect = 0.0;
        double sumSquaredLogEffect = 0.0;

        for (std::size_t i = 0;
            i < sampleCount;
            ++i)
        {
            const auto change =
                ae::sampleNodeKineticChange(
                    statisticalModel,
                    statisticalRandom
                );

            const double logEffect =
                std::log(
                    change.basalProductionMultiplier
                );

            sumLogEffect +=
                logEffect;

            sumSquaredLogEffect +=
                logEffect * logEffect;
        }

        const double denominator =
            static_cast<double>(
                sampleCount
                );

        const double sampleMean =
            sumLogEffect
            / denominator;

        const double sampleVariance =
            sumSquaredLogEffect
            / denominator
            - sampleMean * sampleMean;

        require(
            nearlyEqual(
                sampleMean,
                0.0,
                0.015
            ),
            "Log mutation effects are not centered near zero."
        );

        require(
            nearlyEqual(
                sampleVariance,
                0.25,
                0.015
            ),
            "Log mutation-effect variance is incorrect."
        );


        // --------------------------------------------------------
        // Test 5:
        // Invalid effect scales are rejected.
        // --------------------------------------------------------

        requireThrows(
            []
            {
                ae::RegulatoryMutationEffectModel model{};

                model.nodeBasalProductionLogStdDev =
                    -0.1;

                ae::Random random(1);

                const auto change =
                    ae::sampleNodeKineticChange(
                        model,
                        random
                    );
            },
            "Negative mutation-effect scale was accepted."
        );

        requireThrows(
            []
            {
                ae::RegulatoryMutationEffectModel model{};

                model.interactionFoldChangeLogStdDev =
                    std::numeric_limits<double>::infinity();

                ae::Random random(1);

                const auto change =
                    ae::sampleInteractionParameterChange(
                        model,
                        random
                    );
            },
            "Infinite mutation-effect scale was accepted."
        );


        std::cout
            << "All regulatory mutation effect model tests passed.\n";

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