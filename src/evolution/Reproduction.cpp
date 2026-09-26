#include "alien_evolution/evolution/Reproduction.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ae
{

    namespace
    {

        const Organism& selectFitnessWeightedParent(
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

            double cumulativeFitness =
                0.0;

            for (
                const Organism& organism :
                population.organisms()
                )
            {
                cumulativeFitness +=
                    organism.fitness();

                if (
                    target
                    < cumulativeFitness
                    )
                {
                    return organism;
                }
            }

            // Floating-point defensive fallback.
            for (
                std::size_t i =
                population.size();
                i > 0;
                --i
                )
            {
                const Organism& organism =
                    population.at(
                        i - 1
                    );

                if (
                    organism.fitness()
            > 0.0
                    )
                {
                    return organism;
                }
            }

            throw std::logic_error(
                "No selectable parent found despite positive total fitness."
            );
        }

        const Organism& selectUniformParent(
            const Population& population,
            Random& random
        )
        {
            const std::size_t index =
                static_cast<std::size_t>(
                    random.uniform(
                        0.0,
                        static_cast<double>(
                            population.size()
                            )
                    )
                    );

            const std::size_t safeIndex =
                index < population.size()
                ? index
                : population.size() - 1;

            return population.at(
                safeIndex
            );
        }

    } // namespace

    Population reproducePopulation(
        const Population& parents,
        const std::size_t offspringCount,
        Random& random,
        const RegulatoryMutationGeneratorConfig& mutationConfig,
        const SelectionMode selectionMode
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

        double totalFitness =
            0.0;

        for (
            const Organism& organism :
            parents.organisms()
            )
        {
            if (!organism.hasFitness())
            {
                throw std::logic_error(
                    "Cannot reproduce an unevaluated population."
                );
            }

            const double fitness =
                organism.fitness();

            if (
                !std::isfinite(fitness)
                || fitness < 0.0
                )
            {
                throw std::logic_error(
                    "Population contains invalid fitness."
                );
            }

            totalFitness +=
                fitness;
        }

        if (
            selectionMode
            == SelectionMode::FitnessProportional
            && totalFitness <= 0.0
            )
        {
            // Under the current non-overlapping reproduction model,
            // a population with no reproductive fitness goes extinct.
            return Population(
                std::vector<Organism>{}
            );
        }

        std::vector<Organism> offspring;

        offspring.reserve(
            offspringCount
        );

        for (
            std::size_t i = 0;
            i < offspringCount;
            ++i
            )
        {
            const Organism* parent =
                nullptr;

            switch (selectionMode)
            {
            case SelectionMode::FitnessProportional:

                parent =
                    &selectFitnessWeightedParent(
                        parents,
                        totalFitness,
                        random
                    );

                break;

            case SelectionMode::Uniform:

                parent =
                    &selectUniformParent(
                        parents,
                        random
                    );

                break;

            default:

                throw std::logic_error(
                    "Unknown selection mode."
                );
            }

            RegulatoryMutationGenerationResult mutationResult =
                generateRegulatoryOffspring(
                    parent->regulatoryProgram(),
                    mutationConfig,
                    random
                );

            offspring.emplace_back(
                std::move(
                    mutationResult.offspringProgram
                )
            );
        }

        return Population(
            std::move(
                offspring
            )
        );
    }

} // namespace ae