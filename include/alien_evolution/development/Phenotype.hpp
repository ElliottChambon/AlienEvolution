#pragma once

#include <cstddef>
#include <vector>

namespace ae
{

    class Phenotype
    {
    public:
        Phenotype(std::size_t width, std::size_t height);

        [[nodiscard]] std::size_t width() const;
        [[nodiscard]] std::size_t height() const;

        [[nodiscard]] double materialAt(
            std::size_t x,
            std::size_t y
        ) const;

        void setMaterial(
            std::size_t x,
            std::size_t y,
            double value
        );

        [[nodiscard]] double totalMaterial() const;

    private:
        std::size_t width_;
        std::size_t height_;

        std::vector<double> material_;

        [[nodiscard]] std::size_t index(
            std::size_t x,
            std::size_t y
        ) const;
    };

} // namespace ae