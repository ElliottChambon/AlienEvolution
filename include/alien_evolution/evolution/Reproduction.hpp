#pragma once

#include <cstddef>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/evolution/Population.hpp"
#include "alien_evolution/genetics/Mutation.hpp"

namespace ae
{

    [[nodiscard]] Population reproducePopulation(
        const Population& parents,
        std::size_t offspringCount,
        Random& random,
        const MutationConfig& mutationConfig
    );

} // namespace ae