#include "alien_evolution/evaluation/Fitness.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace ae
{

    double calculateFitness(
        const EnergeticConsequences& energetics
    )
    {
        if (
            !std::isfinite(energetics.resourceAcquisition)
            || !std::isfinite(energetics.maintenanceCost)
            || !std::isfinite(energetics.netEnergy)
            )
        {
            throw std::invalid_argument(
                "Energetic consequences must be finite."
            );
        }

        return std::max(
            0.0,
            energetics.netEnergy
        );
    }

} // namespace ae