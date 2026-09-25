#include "alien_evolution/evaluation/Energetics.hpp"

#include <cmath>
#include <stdexcept>

namespace ae
{

    EnergeticConsequences evaluateEnergetics(
        const PhenotypeMetrics& metrics,
        const Environment& environment,
        const EnergeticsConfig& config
    )
    {
        if (
            !std::isfinite(metrics.totalMaterial)
            || metrics.totalMaterial < 0.0
            || !std::isfinite(metrics.exposedBoundary)
            || metrics.exposedBoundary < 0.0
            )
        {
            throw std::invalid_argument(
                "Phenotype metrics must be finite and non-negative."
            );
        }

        if (
            !std::isfinite(environment.resourceAvailability)
            || environment.resourceAvailability < 0.0
            )
        {
            throw std::invalid_argument(
                "Environmental resource availability must be finite and non-negative."
            );
        }

        if (
            !std::isfinite(config.resourceGainPerBoundary)
            || config.resourceGainPerBoundary < 0.0
            || !std::isfinite(config.maintenanceCostPerMaterial)
            || config.maintenanceCostPerMaterial < 0.0
            )
        {
            throw std::invalid_argument(
                "Energetics parameters must be finite and non-negative."
            );
        }

        EnergeticConsequences consequences{};

        consequences.resourceAcquisition =
            metrics.exposedBoundary
            * environment.resourceAvailability
            * config.resourceGainPerBoundary;

        consequences.maintenanceCost =
            metrics.totalMaterial
            * config.maintenanceCostPerMaterial;

        consequences.netEnergy =
            consequences.resourceAcquisition
            - consequences.maintenanceCost;

        return consequences;
    }

} // namespace ae