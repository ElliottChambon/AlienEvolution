#pragma once

#include "alien_evolution/development/Phenotype.hpp"

namespace ae
{

    struct PhenotypeMetrics
    {
        // Total amount of organismal material in the phenotype.
        double totalMaterial = 0.0;

        // Approximate exposed organism/environment interface.
        //
        // In V0.1 this is measured in grid-edge units from the
        // 2D occupancy field. It is not yet physical surface area.
        double exposedBoundary = 0.0;
    };

    [[nodiscard]] PhenotypeMetrics measurePhenotype(
        const Phenotype& phenotype
    );

} // namespace ae