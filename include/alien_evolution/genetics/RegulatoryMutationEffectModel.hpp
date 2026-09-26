#pragma once

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/genetics/RegulatoryMutation.hpp"

namespace ae
{

    // ------------------------------------------------------------
    // V0.2 quantitative-effect null model.
    //
    // Each positive regulatory parameter is perturbed
    // multiplicatively:
    //
    //     p' = p * exp(epsilon)
    //
    // where:
    //
    //     epsilon ~ Normal(0, sigma)
    //
    // The sigma values are model inputs, not universal
    // biological constants.
    //
    // All values default to zero so that the simulator never
    // silently assumes an arbitrary mutation-effect magnitude.
    // ------------------------------------------------------------

    struct RegulatoryMutationEffectModel
    {
        double nodeBasalProductionLogStdDev = 0.0;

        double nodeDegradationLogStdDev = 0.0;

        double interactionFoldChangeLogStdDev = 0.0;

        double interactionHalfSaturationLogStdDev = 0.0;

        double interactionCooperativityLogStdDev = 0.0;
    };

    [[nodiscard]]
    NodeKineticChange sampleNodeKineticChange(
        const RegulatoryMutationEffectModel& model,
        Random& random
    );

    [[nodiscard]]
    InteractionParameterChange sampleInteractionParameterChange(
        const RegulatoryMutationEffectModel& model,
        Random& random
    );

} // namespace ae