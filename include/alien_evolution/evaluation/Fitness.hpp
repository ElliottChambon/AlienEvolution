#pragma once

#include "alien_evolution/evaluation/Energetics.hpp"

namespace ae
{

    // V0.1 fitness is proportional to energetic surplus available
    // after resource acquisition and maintenance.
    //
    // A non-positive energetic balance produces zero reproductive
    // fitness.
    //
    // This is a deliberately simplified V0.1 model and is not intended
    // as a universal definition of biological fitness.
    [[nodiscard]] double calculateFitness(
        const EnergeticConsequences& energetics
    );

} // namespace ae