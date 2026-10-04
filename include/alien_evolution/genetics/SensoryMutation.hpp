#pragma once

#include <cstddef>
#include <vector>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/genetics/SensoryProgram.hpp"

namespace ae
{
    struct SensoryChannelParameterChange
    {
        double foldChangeMultiplier = 1.0;
        double halfSaturationMultiplier = 1.0;
        double cooperativityMultiplier = 1.0;
    };

    // Index identity is temporary: B5 has fixed channel structure/order.
    // Revisit before structural sensory evolution; no permanent IDs are added.
    struct SensoryChannelParameterMutationEvent
    {
        std::size_t channelIndex = 0;
        SensoryChannelParameterChange change{};
    };

    [[nodiscard]] SensoryProgram applySensoryMutationEvents(
        const SensoryProgram& parent,
        const std::vector<SensoryChannelParameterMutationEvent>& events
    );

    // Expected events per eligible channel per normalized replication interval.
    // Rates and effect scales are explicit null-model inputs, not biological constants.
    struct SensoryMutationRateModel
    {
        double channelParameterPerChannel = 0.0;
    };

    [[nodiscard]] double computeSensoryMutationHazard(
        const SensoryProgram& program,
        const SensoryMutationRateModel& rates
    );

    struct SensoryMutationEffectModel
    {
        double foldChangeLogStdDev = 0.0;
        double halfSaturationLogStdDev = 0.0;
        double cooperativityLogStdDev = 0.0;
    };

    [[nodiscard]] SensoryChannelParameterChange sampleSensoryChannelParameterChange(
        const SensoryMutationEffectModel& model,
        Random& random
    );

    struct SensoryMutationGeneratorConfig
    {
        SensoryMutationRateModel rates{};
        SensoryMutationEffectModel quantitativeEffects{};
        std::size_t safetyEventLimit = 1000000;
    };

    struct TimedSensoryMutationEvent
    {
        double replicationProgress = 0.0;
        SensoryChannelParameterMutationEvent event{};
    };

    struct SensoryMutationGenerationResult
    {
        SensoryProgram offspringProgram;
        std::vector<TimedSensoryMutationEvent> events;
    };

    // Constant channel hazard with uniform target selection is a Poisson/null
    // model. Zero hazard consumes no random draws. No target mapping is repaired.
    [[nodiscard]] SensoryMutationGenerationResult generateSensoryOffspring(
        const SensoryProgram& parent,
        const SensoryMutationGeneratorConfig& config,
        Random& random
    );
} // namespace ae
