#include "alien_evolution/evaluation/PhenotypeMetrics.hpp"

#include <cmath>

namespace ae
{

    PhenotypeMetrics measurePhenotype(
        const Phenotype& phenotype
    )
    {
        PhenotypeMetrics metrics{};

        metrics.totalMaterial =
            phenotype.totalMaterial();

        double exposedBoundary = 0.0;

        const std::size_t width =
            phenotype.width();

        const std::size_t height =
            phenotype.height();

        for (std::size_t y = 0; y < height; ++y)
        {
            for (std::size_t x = 0; x < width; ++x)
            {
                const double material =
                    phenotype.materialAt(x, y);

                // External left boundary.
                if (x == 0)
                {
                    exposedBoundary += material;
                }

                // External right boundary.
                if (x + 1 == width)
                {
                    exposedBoundary += material;
                }

                // External top boundary.
                if (y == 0)
                {
                    exposedBoundary += material;
                }

                // External bottom boundary.
                if (y + 1 == height)
                {
                    exposedBoundary += material;
                }

                // Internal vertical interface.
                if (x + 1 < width)
                {
                    const double neighbor =
                        phenotype.materialAt(x + 1, y);

                    exposedBoundary +=
                        std::abs(material - neighbor);
                }

                // Internal horizontal interface.
                if (y + 1 < height)
                {
                    const double neighbor =
                        phenotype.materialAt(x, y + 1);

                    exposedBoundary +=
                        std::abs(material - neighbor);
                }
            }
        }

        metrics.exposedBoundary =
            exposedBoundary;

        return metrics;
    }

} // namespace ae