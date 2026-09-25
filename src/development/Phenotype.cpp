#include "alien_evolution/development/Phenotype.hpp"

#include <algorithm>
#include <stdexcept>

namespace ae
{

    Phenotype::Phenotype(
        const std::size_t width,
        const std::size_t height
    )
        : width_(width),
        height_(height),
        material_(width* height, 0.0)
    {
        if (width == 0 || height == 0)
        {
            throw std::invalid_argument(
                "Phenotype dimensions must be greater than zero."
            );
        }
    }

    std::size_t Phenotype::width() const
    {
        return width_;
    }

    std::size_t Phenotype::height() const
    {
        return height_;
    }

    double Phenotype::materialAt(
        const std::size_t x,
        const std::size_t y
    ) const
    {
        return material_.at(index(x, y));
    }

    void Phenotype::setMaterial(
        const std::size_t x,
        const std::size_t y,
        const double value
    )
    {
        material_.at(index(x, y)) =
            std::clamp(value, 0.0, 1.0);
    }

    double Phenotype::totalMaterial() const
    {
        double total = 0.0;

        for (const double value : material_)
        {
            total += value;
        }

        return total;
    }

    std::size_t Phenotype::index(
        const std::size_t x,
        const std::size_t y
    ) const
    {
        if (x >= width_ || y >= height_)
        {
            throw std::out_of_range(
                "Phenotype grid coordinate out of range."
            );
        }

        return y * width_ + x;
    }

} // namespace ae