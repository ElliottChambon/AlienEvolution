#include "alien_evolution/evolution/Population.hpp"

#include <stdexcept>
#include <utility>

namespace ae
{

    Population::Population(
        const std::size_t populationSize,
        const Genome& founderGenome,
        Random& random,
        const MutationConfig& initialVariation
    )
    {
        if (populationSize == 0)
        {
            throw std::invalid_argument(
                "Initial population size must be greater than zero."
            );
        }

        organisms_.reserve(populationSize);

        for (std::size_t i = 0; i < populationSize; ++i)
        {
            Genome genome =
                mutateGenome(
                    founderGenome,
                    random,
                    initialVariation
                );

            organisms_.emplace_back(
                std::move(genome)
            );
        }
    }

    Population::Population(
        std::vector<Organism> organisms
    )
        : organisms_(std::move(organisms))
    {}

    std::size_t Population::size() const
    {
        return organisms_.size();
    }

    bool Population::empty() const
    {
        return organisms_.empty();
    }

    const Organism& Population::at(
        const std::size_t index
    ) const
    {
        return organisms_.at(index);
    }

    Organism& Population::at(
        const std::size_t index
    )
    {
        return organisms_.at(index);
    }

    const std::vector<Organism>&
        Population::organisms() const
    {
        return organisms_;
    }

} // namespace ae