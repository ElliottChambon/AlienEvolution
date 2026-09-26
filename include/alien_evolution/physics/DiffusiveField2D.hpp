#pragma once

#include <cstddef>
#include <vector>

namespace ae
{

    struct DiffusiveField2DConfig
    {
        // Distance between neighboring grid locations.
        //
        // The field equations are unit-agnostic as long as all
        // quantities use a consistent system of units.
        double gridSpacing = 1.0;

        // Diffusion coefficient:
        //
        // units = length^2 / time
        double diffusionCoefficient = 0.0;

        // First-order degradation / removal rate:
        //
        // units = 1 / time
        double decayRate = 0.0;
    };


    class DiffusiveField2D
    {
    public:
        DiffusiveField2D(
            std::size_t width,
            std::size_t height,
            DiffusiveField2DConfig config
        );

        [[nodiscard]]
        std::size_t width() const;

        [[nodiscard]]
        std::size_t height() const;

        [[nodiscard]]
        double concentrationAt(
            std::size_t x,
            std::size_t y
        ) const;

        [[nodiscard]]
        double sourceRateAt(
            std::size_t x,
            std::size_t y
        ) const;

        void setConcentration(
            std::size_t x,
            std::size_t y,
            double concentration
        );

        void setSourceRate(
            std::size_t x,
            std::size_t y,
            double sourceRate
        );

        [[nodiscard]]
        double totalAmount() const;

        // Advance:
        //
        // dc/dt = D * laplacian(c)
        //         - k*c
        //         + s
        //
        // Diffusion uses an explicit finite-volume-like nearest-neighbor
        // update with no-flux boundaries.
        //
        // Local first-order decay and constant local source are then
        // integrated analytically over the time step.
        void step(
            double timeStep
        );

    private:
        std::size_t width_;
        std::size_t height_;

        DiffusiveField2DConfig config_;

        std::vector<double> concentration_;
        std::vector<double> sourceRate_;

        [[nodiscard]]
        std::size_t index(
            std::size_t x,
            std::size_t y
        ) const;

        void validateCoordinates(
            std::size_t x,
            std::size_t y
        ) const;
    };

} // namespace ae