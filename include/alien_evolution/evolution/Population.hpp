#pragma once

#include <cstddef>
#include <vector>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/evolution/Organism.hpp"
#include "alien_evolution/genetics/RegulatoryMutationGenerator.hpp"
#include "alien_evolution/genetics/RegulatoryProgram.hpp"

namespace ae
{

    class Population
    {
    public:
        Population(
            std::size_t populationSize,
            const RegulatoryProgram& founderProgram,
            Random& random,
            const RegulatoryMutationGeneratorConfig& initialVariation
        );

        explicit Population(
            std::vector<Organism> organisms
        );

        [[nodiscard]]
        std::size_t size() const;

        [[nodiscard]]
        bool empty() const;

        [[nodiscard]]
        const Organism& at(
            std::size_t index
        ) const;

        [[nodiscard]]
        Organism& at(
            std::size_t index
        );

        [[nodiscard]]
        const std::vector<Organism>& organisms() const;

    private:
        std::vector<Organism> organisms_;
    };

} // namespace ae