#pragma once

#include <cstddef>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/evolution/Population.hpp"
#include "alien_evolution/genetics/RegulatoryMutationGenerator.hpp"

namespace ae
{

    enum class SelectionMode
    {
        FitnessProportional,
        Uniform
    };

    [[nodiscard]]
    Population reproducePopulation(
        const Population& parents,
        std::size_t offspringCount,
        Random& random,
        const RegulatoryMutationGeneratorConfig& mutationConfig,
        SelectionMode selectionMode
    );

} // namespace ae