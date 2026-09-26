#include "alien_evolution/physics/DiffusiveField2D.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ae
{

    DiffusiveField2D::DiffusiveField2D(
        const std::size_t width,
        const std::size_t height,
        DiffusiveField2DConfig config
    )
        : width_(width),
        height_(height),
        config_(std::move(config)),
        concentration_(
            width* height,
            0.0
        ),
        sourceRate_(
            width* height,
            0.0
        )
    {
        if (
            width_ == 0
            || height_ == 0
            )
        {
            throw std::invalid_argument(
                "Diffusive field dimensions must be positive."
            );
        }

        if (
            !std::isfinite(
                config_.gridSpacing
            )
            || config_.gridSpacing <= 0.0
            )
        {
            throw std::invalid_argument(
                "Diffusive field grid spacing must be finite and positive."
            );
        }

        if (
            !std::isfinite(
                config_.diffusionCoefficient
            )
            || config_.diffusionCoefficient < 0.0
            )
        {
            throw std::invalid_argument(
                "Diffusion coefficient must be finite and nonnegative."
            );
        }

        if (
            !std::isfinite(
                config_.decayRate
            )
            || config_.decayRate < 0.0
            )
        {
            throw std::invalid_argument(
                "Decay rate must be finite and nonnegative."
            );
        }
    }


    std::size_t DiffusiveField2D::width() const
    {
        return width_;
    }


    std::size_t DiffusiveField2D::height() const
    {
        return height_;
    }


    std::size_t DiffusiveField2D::index(
        const std::size_t x,
        const std::size_t y
    ) const
    {
        return
            y * width_
            + x;
    }


    void DiffusiveField2D::validateCoordinates(
        const std::size_t x,
        const std::size_t y
    ) const
    {
        if (
            x >= width_
            || y >= height_
            )
        {
            throw std::out_of_range(
                "Diffusive field coordinates are outside the domain."
            );
        }
    }


    double DiffusiveField2D::concentrationAt(
        const std::size_t x,
        const std::size_t y
    ) const
    {
        validateCoordinates(
            x,
            y
        );

        return
            concentration_[
                index(
                    x,
                    y
                )
            ];
    }


    double DiffusiveField2D::sourceRateAt(
        const std::size_t x,
        const std::size_t y
    ) const
    {
        validateCoordinates(
            x,
            y
        );

        return
            sourceRate_[
                index(
                    x,
                    y
                )
            ];
    }


    void DiffusiveField2D::setConcentration(
        const std::size_t x,
        const std::size_t y,
        const double concentration
    )
    {
        validateCoordinates(
            x,
            y
        );

        if (
            !std::isfinite(
                concentration
            )
            || concentration < 0.0
            )
        {
            throw std::invalid_argument(
                "Field concentration must be finite and nonnegative."
            );
        }

        concentration_[
            index(
                x,
                y
            )
        ] =
            concentration;
    }


    void DiffusiveField2D::setSourceRate(
        const std::size_t x,
        const std::size_t y,
        const double sourceRate
    )
    {
        validateCoordinates(
            x,
            y
        );

        if (
            !std::isfinite(
                sourceRate
            )
            || sourceRate < 0.0
            )
        {
            throw std::invalid_argument(
                "Field source rate must be finite and nonnegative."
            );
        }

        sourceRate_[
            index(
                x,
                y
            )
        ] =
            sourceRate;
    }


    double DiffusiveField2D::totalAmount() const
    {
        double concentrationSum =
            0.0;

        for (
            const double concentration :
        concentration_
            )
        {
            concentrationSum +=
                concentration;
        }

        const double cellArea =
            config_.gridSpacing
            * config_.gridSpacing;

        return
            concentrationSum
            * cellArea;
    }


    void DiffusiveField2D::step(
        const double timeStep
    )
    {
        if (
            !std::isfinite(
                timeStep
            )
            || timeStep <= 0.0
            )
        {
            throw std::invalid_argument(
                "Diffusive field time step must be finite and positive."
            );
        }


        const double dx =
            config_.gridSpacing;

        const double diffusionNumber =
            config_.diffusionCoefficient
            * timeStep
            / (dx * dx);


        // For the explicit 2D nearest-neighbor diffusion scheme,
        // r <= 1/4 keeps the interior update coefficients
        // nonnegative.
        if (
            diffusionNumber
            > 0.25
            )
        {
            throw std::invalid_argument(
                "Diffusive field time step violates the explicit 2D diffusion stability limit."
            );
        }


        std::vector<double> afterDiffusion =
            concentration_;


        // --------------------------------------------------------
        // Diffusion
        //
        // Flux is exchanged only with neighbors that actually
        // exist inside the domain.
        //
        // Missing external neighbors therefore contribute zero
        // normal flux: a no-flux boundary.
        // --------------------------------------------------------

        for (
            std::size_t y = 0;
            y < height_;
            ++y
            )
        {
            for (
                std::size_t x = 0;
                x < width_;
                ++x
                )
            {
                const std::size_t currentIndex =
                    index(
                        x,
                        y
                    );

                const double current =
                    concentration_[
                        currentIndex
                    ];

                double neighborDifferenceSum =
                    0.0;


                if (x > 0)
                {
                    neighborDifferenceSum +=
                        concentration_[
                            index(
                                x - 1,
                                y
                            )
                        ]
                        - current;
                }

                if (x + 1 < width_)
                {
                    neighborDifferenceSum +=
                        concentration_[
                            index(
                                x + 1,
                                y
                            )
                        ]
                        - current;
                }

                if (y > 0)
                {
                    neighborDifferenceSum +=
                        concentration_[
                            index(
                                x,
                                y - 1
                            )
                        ]
                        - current;
                }

                if (y + 1 < height_)
                {
                    neighborDifferenceSum +=
                        concentration_[
                            index(
                                x,
                                y + 1
                            )
                        ]
                        - current;
                }


                afterDiffusion[
                    currentIndex
                ] =
                    current
                        + diffusionNumber
                        * neighborDifferenceSum;


                    if (
                        !std::isfinite(
                            afterDiffusion[
                                currentIndex
                            ]
                        )
                        ||
                        afterDiffusion[
                            currentIndex
                        ]
                        < 0.0
                        )
                    {
                        throw std::runtime_error(
                            "Diffusive field produced an invalid concentration during diffusion."
                        );
                    }
            }
        }


        // --------------------------------------------------------
        // Local source + first-order decay
        //
        // dc/dt = s - k*c
        //
        // For constant s and k over one step:
        //
        // c(t+dt) =
        //     c(t)e^(-k dt)
        //     + (s/k)(1-e^(-k dt))
        //
        // This avoids introducing an additional decay-related
        // explicit-Euler stability restriction.
        // --------------------------------------------------------

        if (
            config_.decayRate > 0.0
            )
        {
            const double decayFactor =
                std::exp(
                    -config_.decayRate
                    * timeStep
                );

            const double sourceMultiplier =
                (
                    1.0
                    - decayFactor
                    )
                / config_.decayRate;


            for (
                std::size_t i = 0;
                i < concentration_.size();
                ++i
                )
            {
                concentration_[i] =
                    afterDiffusion[i]
                    * decayFactor
                    +
                    sourceRate_[i]
                    * sourceMultiplier;


                if (
                    !std::isfinite(
                        concentration_[i]
                    )
                    || concentration_[i] < 0.0
                    )
                {
                    throw std::runtime_error(
                        "Diffusive field produced an invalid concentration during source-decay integration."
                    );
                }
            }
        }
        else
        {
            for (
                std::size_t i = 0;
                i < concentration_.size();
                ++i
                )
            {
                concentration_[i] =
                    afterDiffusion[i]
                    +
                    sourceRate_[i]
                    * timeStep;


                if (
                    !std::isfinite(
                        concentration_[i]
                    )
                    || concentration_[i] < 0.0
                    )
                {
                    throw std::runtime_error(
                        "Diffusive field produced an invalid concentration during source integration."
                    );
                }
            }
        }
    }

} // namespace ae