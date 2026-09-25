#include "alien_evolution/development/Development.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace ae
{

    Development::Development(
        const std::size_t width,
        const std::size_t height,
        const std::size_t steps
    )
        : width_(width),
        height_(height),
        steps_(steps)
    {
        if (width == 0 || height == 0)
        {
            throw std::invalid_argument(
                "Development grid dimensions must be greater than zero."
            );
        }

        if (steps == 0)
        {
            throw std::invalid_argument(
                "Development must contain at least one step."
            );
        }
    }

    Phenotype Development::develop(
        const Genome& genome,
        const Environment& environment
    ) const
    {
        Phenotype phenotype(width_, height_);

        // V0.1 developmental seed.
        phenotype.setMaterial(
            width_ / 2,
            height_ / 2,
            1.0
        );

        for (std::size_t step = 0; step < steps_; ++step)
        {
            Phenotype next = phenotype;

            for (std::size_t y = 0; y < height_; ++y)
            {
                for (std::size_t x = 0; x < width_; ++x)
                {
                    const double currentMaterial =
                        phenotype.materialAt(x, y);

                    const double signal =
                        localSignal(
                            phenotype,
                            x,
                            y,
                            genome.lambda
                        );

                    // Existing organismal material is required to generate
                    // developmental signaling. Environmental resources
                    // modulate that signal rather than independently
                    // nucleating new tissue.
                    const double regulatorySignal =
                        signal
                        * (
                            genome.alphaR
                            + genome.alphaE
                            * environment.resourceAvailability
                            )
                        - genome.theta;

                    // A true developmental threshold prevents weak,
                    // long-range signals from slowly filling empty space.
                    if (regulatorySignal <= 0.0)
                    {
                        continue;
                    }

                    const double active =
                        activation(
                            regulatorySignal,
                            genome.beta
                        );

                    const double availableSpace =
                        1.0 - currentMaterial;

                    if (availableSpace <= 0.0)
                    {
                        continue;
                    }

                    const double resourceFactor =
                        std::max(
                            environment.resourceAvailability,
                            0.0
                        );

                    const double cost =
                        std::max(
                            genome.metabolicCost,
                            1.0e-9
                        );

                    const double deposition =
                        genome.growthRate
                        * active
                        * availableSpace
                        * resourceFactor
                        / cost;

                    next.setMaterial(
                        x,
                        y,
                        currentMaterial + deposition
                    );
                }
            }

            phenotype = std::move(next);
        }

        return phenotype;
    }

    double Development::localSignal(
        const Phenotype& phenotype,
        const std::size_t x,
        const std::size_t y,
        const double lambda
    ) const
    {
        const double safeLambda =
            std::max(lambda, 1.0e-9);

        double signal = 0.0;

        // V0.1 uses a local 3x3 developmental neighborhood.
        //
        // This prevents material from appearing disconnected from the
        // existing organism while still allowing the developmental front
        // to propagate outward.
        for (int offsetY = -1; offsetY <= 1; ++offsetY)
        {
            for (int offsetX = -1; offsetX <= 1; ++offsetX)
            {
                if (offsetX == 0 && offsetY == 0)
                {
                    continue;
                }

                const int sourceX =
                    static_cast<int>(x) + offsetX;

                const int sourceY =
                    static_cast<int>(y) + offsetY;

                if (
                    sourceX < 0
                    || sourceY < 0
                    || sourceX >= static_cast<int>(phenotype.width())
                    || sourceY >= static_cast<int>(phenotype.height())
                    )
                {
                    continue;
                }

                const double distance =
                    std::sqrt(
                        static_cast<double>(
                            offsetX * offsetX
                            + offsetY * offsetY
                            )
                    );

                const double weight =
                    std::exp(
                        -distance / safeLambda
                    );

                signal +=
                    weight
                    * phenotype.materialAt(
                        static_cast<std::size_t>(sourceX),
                        static_cast<std::size_t>(sourceY)
                    );
            }
        }

        return signal;
    }

    double Development::activation(
        const double regulatorySignal,
        const double beta
    )
    {
        const double exponent =
            std::clamp(
                -beta * regulatorySignal,
                -60.0,
                60.0
            );

        return 1.0 / (1.0 + std::exp(exponent));
    }

} // namespace ae