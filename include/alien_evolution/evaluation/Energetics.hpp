#pragma once

#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/evaluation/PhenotypeMetrics.hpp"

namespace ae
{

    struct EnergeticsConfig
    {
        // Resource acquired per unit exposed boundary
        // when environmental resource availability is 1.0.
        double resourceGainPerBoundary = 1.0;

        // Maintenance cost per unit organismal material.
        double maintenanceCostPerMaterial = 0.10;
    };

    struct EnergeticConsequences
    {
        double resourceAcquisition = 0.0;
        double maintenanceCost = 0.0;

        // May be negative.
        double netEnergy = 0.0;
    };

    [[nodiscard]] EnergeticConsequences evaluateEnergetics(
        const PhenotypeMetrics& metrics,
        const Environment& environment,
        const EnergeticsConfig& config
    );

} // namespace ae