#pragma once

#include <cstddef>

#include "alien_evolution/development/Phenotype.hpp"
#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/genetics/Genome.hpp"

namespace ae
{

    class Development
    {
    public:
        Development(
            std::size_t width,
            std::size_t height,
            std::size_t steps
        );

        [[nodiscard]] Phenotype develop(
            const Genome& genome,
            const Environment& environment
        ) const;

    private:
        std::size_t width_;
        std::size_t height_;
        std::size_t steps_;

        [[nodiscard]] double localSignal(
            const Phenotype& phenotype,
            std::size_t x,
            std::size_t y,
            double lambda
        ) const;

        [[nodiscard]] static double activation(
            double regulatorySignal,
            double beta
        );
    };

} // namespace ae