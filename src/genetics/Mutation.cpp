#include "alien_evolution/genetics/Mutation.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace ae
{

    namespace
    {

        double mutatePositiveParameter(
            const double value,
            Random& random,
            const MutationConfig& config
        )
        {
            constexpr double minimumPositive = 1.0e-9;

            const double safeValue =
                std::max(value, minimumPositive);

            if (!random.chance(config.probabilityPerParameter))
            {
                return safeValue;
            }

            const double mutation =
                random.normal(
                    0.0,
                    config.logStandardDeviation
                );

            return std::exp(
                std::log(safeValue) + mutation
            );
        }

    } // namespace

    Genome mutateGenome(
        const Genome& parent,
        Random& random,
        const MutationConfig& config
    )
    {
        if (
            config.probabilityPerParameter < 0.0
            || config.probabilityPerParameter > 1.0
            )
        {
            throw std::invalid_argument(
                "Mutation probability must lie in [0, 1]."
            );
        }

        if (config.logStandardDeviation < 0.0)
        {
            throw std::invalid_argument(
                "Mutation standard deviation cannot be negative."
            );
        }

        Genome child = parent;

        child.alphaR =
            mutatePositiveParameter(
                parent.alphaR,
                random,
                config
            );

        child.alphaE =
            mutatePositiveParameter(
                parent.alphaE,
                random,
                config
            );

        child.theta =
            mutatePositiveParameter(
                parent.theta,
                random,
                config
            );

        child.lambda =
            mutatePositiveParameter(
                parent.lambda,
                random,
                config
            );

        child.beta =
            mutatePositiveParameter(
                parent.beta,
                random,
                config
            );

        child.growthRate =
            mutatePositiveParameter(
                parent.growthRate,
                random,
                config
            );

        child.metabolicCost =
            mutatePositiveParameter(
                parent.metabolicCost,
                random,
                config
            );

        return child;
    }

} // namespace ae