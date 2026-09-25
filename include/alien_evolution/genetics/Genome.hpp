#pragma once

namespace ae
{

    struct Genome
    {
        // Developmental regulation parameters.
        double alphaR = 1.0;
        double alphaE = 1.0;

        // Developmental threshold.
        double theta = 0.5;

        // Characteristic spatial/developmental scale.
        double lambda = 1.0;

        // Regulatory response strength.
        double beta = 1.0;

        // Growth parameter.
        double growthRate = 1.0;

        // Metabolic/material cost parameter.
        double metabolicCost = 1.0;
    };

} // namespace ae