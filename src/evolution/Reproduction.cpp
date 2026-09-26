#include "alien_evolution/evolution/Reproduction.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ae
{

    namespace
    {

        const Organism& selectParent(
            const Population& population,
            const double totalFitness,
            Random& random
        )
        {
            const double target =
                random.uniform(
                    0.0,
                    totalFitness
                );

            double cumulativeFitness = 0.0;

            for (const Organism& organism :
                population.organisms())
            {
                cumulativeFitness += organism.fitness();

                if (target < cumulativeFitness)
                {
                    return organism;
                }
            }

            // Floating-point fallback.
            for (std::size_t i = population.size();
                i > 0;
                --i)
            {
                const Organism& organism =
                    population.at(i - 1);

                if (organism.fitness() > 0.0)
                {
                    return organism;
                }
            }

            throw std::logic_error(
                "No selectable parent found despite positive total fitness."
            );
        }

    } // namespace

    Population reproducePopulation(
        const Population& parents,
        const std::size_t offspringCount,
        Random& random,
        const MutationConfig& mutationConfig
    )
    {
        if (offspringCount == 0)
        {
            return Population(
                std::vector<Organism>{}
            );
        }

        if (parents.empty())
        {
            return Population(
                std::vector<Organism>{}
            );
        }

        double totalFitness = 0.0;

        for (const Organism& organism :
            parents.organisms())
        {
            if (!organism.hasFitness())
            {
                throw std::logic_error(
                    "Cannot reproduce an unevaluated population."
                );
            }

            const double fitness =
                organism.fitness();

            if (!std::isfinite(fitness) || fitness < 0.0)
            {
                throw std::logic_error(
                    "Population contains invalid fitness."
                );
            }

            totalFitness += fitness;
        }

        // No organism possesses reproductive surplus.
        // The lineage/population therefore goes extinct.
        if (totalFitness <= 0.0)
        {
            return Population(
                std::vector<Organism>{}
            );
        }

        std::vector<Organism> offspring;
        offspring.reserve(offspringCount);

        for (std::size_t i = 0;
            i < offspringCount;
            ++i)
        {
            const Organism& parent =
                selectParent(
                    parents,
                    totalFitness,
                    random
                );

            Genome childGenome =
                mutateGenome(
                    parent.genome(),
                    random,
                    mutationConfig
                );

            offspring.emplace_back(
                std::move(childGenome)
            );
        }

        return Population(
            std::move(offspring)
        );
    }

} // namespace ae