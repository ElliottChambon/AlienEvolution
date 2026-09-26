#pragma once

#include <cstdint>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/genetics/RegulatoryMutationRateModel.hpp"

namespace ae
{

    struct RegulatoryMutationCounts
    {
        std::uint64_t nodeKinetic = 0;

        std::uint64_t interactionParameter = 0;

        std::uint64_t interactionGain = 0;

        std::uint64_t interactionLoss = 0;

        std::uint64_t regulatoryUnitDuplication = 0;

        std::uint64_t regulatoryNodeLoss = 0;

        [[nodiscard]]
        std::uint64_t total() const;
    };

    // V0.2 stochastic null model.
    //
    // Each mutation class is sampled as an independent Poisson
    // process whose mean is the expected event count computed
    // from the mutation-rate model.
    //
    // This independence assumption is deliberately replaceable.
    // It is not claimed to capture mutation clustering,
    // sequence-context effects, linked damage, or other
    // mechanistic correlations.
    [[nodiscard]]
    RegulatoryMutationCounts sampleRegulatoryMutationCounts(
        const RegulatoryMutationHazards& hazards,
        Random& random
    );

} // namespace ae