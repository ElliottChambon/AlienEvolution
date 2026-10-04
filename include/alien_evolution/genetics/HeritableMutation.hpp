#pragma once

#include "alien_evolution/genetics/HeritableProgram.hpp"
#include "alien_evolution/genetics/RegulatoryMutationGenerator.hpp"
#include "alien_evolution/genetics/SensoryMutation.hpp"

namespace ae
{
    struct HeritableMutationConfig
    {
        RegulatoryMutationGeneratorConfig regulatory{};
        SensoryMutationGeneratorConfig sensory{};
    };

    struct HeritableMutationGenerationResult
    {
        HeritableProgram offspringProgram;
        std::vector<TimedRegulatoryMutationEvent> regulatoryEvents;
        std::vector<TimedSensoryMutationEvent> sensoryEvents;
    };

    // Regulatory then sensory generation. Zero sensory hazard preserves the
    // historical regulatory outcome and RNG state; no cross-component repair.
    [[nodiscard]] HeritableMutationGenerationResult generateHeritableOffspring(
        const HeritableProgram& parent,
        const HeritableMutationConfig& config,
        Random& random
    );
} // namespace ae
