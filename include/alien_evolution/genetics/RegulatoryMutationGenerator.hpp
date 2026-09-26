#pragma once

#include <cstddef>
#include <vector>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/genetics/RegulatoryMutationEffectModel.hpp"
#include "alien_evolution/genetics/RegulatoryMutationEvent.hpp"
#include "alien_evolution/genetics/RegulatoryMutationRateModel.hpp"
#include "alien_evolution/genetics/RegulatoryProgram.hpp"

namespace ae
{

    // ------------------------------------------------------------
    // V0.2 null model for initializing a newly gained interaction.
    //
    // foldChange is sampled around the neutral value 1.0 in
    // logarithmic space.
    //
    // halfSaturation and cooperativity are sampled around explicit
    // positive reference values.
    //
    // These values are model assumptions, not biological constants.
    // ------------------------------------------------------------

    struct RegulatoryInteractionGainModel
    {
        double foldChangeLogStdDev = 0.0;

        double referenceHalfSaturation = 1.0;
        double halfSaturationLogStdDev = 0.0;

        double referenceCooperativity = 1.0;
        double cooperativityLogStdDev = 0.0;
    };

    struct RegulatoryMutationGeneratorConfig
    {
        RegulatoryMutationRateModel rates{};

        RegulatoryMutationEffectModel quantitativeEffects{};

        RegulatoryInteractionGainModel interactionGain{};

        // Computational guard only.
        //
        // If a configuration produces an unexpectedly explosive
        // number of events, throw rather than silently truncating
        // evolution and biasing the simulation.
        std::size_t safetyEventLimit = 1000000;
    };

    struct TimedRegulatoryMutationEvent
    {
        // Dimensionless position through one replication interval:
        //
        // 0.0 = beginning
        // 1.0 = end
        double replicationProgress = 0.0;

        RegulatoryMutationEvent event;
    };

    struct RegulatoryMutationGenerationResult
    {
        RegulatoryProgram offspringProgram;

        std::vector<TimedRegulatoryMutationEvent> events;
    };

    // Generates and immediately applies mutation events while moving
    // through one normalized replication interval.
    //
    // Hazards are recomputed after every structural change, so later
    // events are generated from the current regulatory topology.
    [[nodiscard]]
    RegulatoryMutationGenerationResult generateRegulatoryOffspring(
        const RegulatoryProgram& parent,
        const RegulatoryMutationGeneratorConfig& config,
        Random& random
    );

} // namespace ae