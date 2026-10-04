#pragma once

#include <cstddef>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/evolution/Population.hpp"
#include "alien_evolution/genetics/RegulatoryMutationGenerator.hpp"
#include "alien_evolution/genetics/HeritableMutation.hpp"

namespace ae
{

    enum class SelectionMode
    {
        FitnessProportional,
        Uniform
    };

    // Opt-in compositional mutation. Config precedes RNG to keep legacy
    // calls with an empty braced regulatory config unambiguous.
    [[nodiscard]]
    Population reproducePopulation(
        const Population& parents,
        std::size_t offspringCount,
        const HeritableMutationConfig& mutationConfig,
        Random& random,
        SelectionMode selectionMode
    );

    [[nodiscard]]
    Population reproducePopulation(
        const Population& parents,
        std::size_t offspringCount,
        Random& random,
        const RegulatoryMutationGeneratorConfig& mutationConfig,
        SelectionMode selectionMode
    );

} // namespace ae
