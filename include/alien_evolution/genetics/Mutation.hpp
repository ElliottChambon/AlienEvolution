#pragma once

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/genetics/Genome.hpp"

namespace ae
{

    struct MutationConfig
    {
        // Probability that each genome parameter mutates during reproduction.
        double probabilityPerParameter = 0.10;

        // Standard deviation of mutation in logarithmic parameter space.
        double logStandardDeviation = 0.10;
    };

    [[nodiscard]] Genome mutateGenome(
        const Genome& parent,
        Random& random,
        const MutationConfig& config
    );

} // namespace ae