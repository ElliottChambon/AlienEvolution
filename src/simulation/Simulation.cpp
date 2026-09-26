#include "alien_evolution/simulation/Simulation.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "alien_evolution/evaluation/Fitness.hpp"
#include "alien_evolution/evaluation/PhenotypeMetrics.hpp"
#include "alien_evolution/evolution/Reproduction.hpp"

namespace ae
{

    Simulation::Simulation(
        Environment environment,
        Genome founderGenome,
        SimulationConfig config,
        const std::uint64_t seed
    )
        : environment_(std::move(environment)),
        config_(std::move(config)),
        random_(seed),
        development_(
            config_.developmentWidth,
            config_.developmentHeight,
            config_.developmentSteps
        ),
        population_(
            config_.populationSize,
            founderGenome,
            random_,
            config_.initialVariation
        )
    {
        if (config_.populationSize == 0)
        {
            throw std::invalid_argument(
                "Simulation population size must be greater than zero."
            );
        }
    }

    const Population& Simulation::population() const
    {
        return population_;
    }

    const Environment& Simulation::environment() const
    {
        return environment_;
    }

    std::size_t Simulation::generation() const
    {
        return generation_;
    }

    std::uint64_t Simulation::seed() const
    {
        return random_.seed();
    }

    bool Simulation::extinct() const
    {
        return population_.empty();
    }

    GenerationStatistics Simulation::step()
    {
        GenerationStatistics statistics =
            evaluatePopulation();

        if (!population_.empty())
        {
            population_ =
                reproducePopulation(
                    population_,
                    config_.populationSize,
                    random_,
                    config_.offspringMutation,
                    config_.selectionMode
                );
        }

        ++generation_;

        return statistics;
    }

    std::vector<GenerationStatistics> Simulation::run(
        const std::size_t generationCount
    )
    {
        std::vector<GenerationStatistics> history;
        history.reserve(generationCount);

        for (std::size_t i = 0;
            i < generationCount;
            ++i)
        {
            if (population_.empty())
            {
                break;
            }

            history.push_back(
                step()
            );
        }

        return history;
    }

    GenerationStatistics Simulation::evaluatePopulation()
    {
        GenerationStatistics statistics{};

        statistics.generation =
            generation_;

        statistics.populationSize =
            population_.size();

        if (population_.empty())
        {
            return statistics;
        }

        double totalFitness = 0.0;
        double maximumFitness = 0.0;

        double totalMaterial = 0.0;
        double totalBoundary = 0.0;
        double totalNetEnergy = 0.0;

        GenomeMeans genomeTotals{};

        for (std::size_t i = 0;
            i < population_.size();
            ++i)
        {
            Organism& organism =
                population_.at(i);

            const Phenotype phenotype =
                development_.develop(
                    organism.genome(),
                    environment_
                );

            const PhenotypeMetrics metrics =
                measurePhenotype(
                    phenotype
                );

            const EnergeticConsequences energetics =
                evaluateEnergetics(
                    metrics,
                    environment_,
                    config_.energetics
                );

            const double fitness =
                calculateFitness(
                    energetics
                );

            organism.setPhenotype(
                phenotype
            );

            organism.setFitness(
                fitness
            );

            totalFitness += fitness;

            maximumFitness =
                std::max(
                    maximumFitness,
                    fitness
                );

            totalMaterial +=
                metrics.totalMaterial;

            totalBoundary +=
                metrics.exposedBoundary;

            totalNetEnergy +=
                energetics.netEnergy;

            const Genome& genome =
                organism.genome();

            genomeTotals.alphaR +=
                genome.alphaR;

            genomeTotals.alphaE +=
                genome.alphaE;

            genomeTotals.theta +=
                genome.theta;

            genomeTotals.lambda +=
                genome.lambda;

            genomeTotals.beta +=
                genome.beta;

            genomeTotals.growthRate +=
                genome.growthRate;

            genomeTotals.metabolicCost +=
                genome.metabolicCost;
        }

        const double count =
            static_cast<double>(
                population_.size()
                );

        statistics.meanFitness =
            totalFitness / count;

        statistics.maximumFitness =
            maximumFitness;

        statistics.meanMaterial =
            totalMaterial / count;

        statistics.meanBoundary =
            totalBoundary / count;

        statistics.meanNetEnergy =
            totalNetEnergy / count;

        statistics.meanGenome.alphaR =
            genomeTotals.alphaR / count;

        statistics.meanGenome.alphaE =
            genomeTotals.alphaE / count;

        statistics.meanGenome.theta =
            genomeTotals.theta / count;

        statistics.meanGenome.lambda =
            genomeTotals.lambda / count;

        statistics.meanGenome.beta =
            genomeTotals.beta / count;

        statistics.meanGenome.growthRate =
            genomeTotals.growthRate / count;

        statistics.meanGenome.metabolicCost =
            genomeTotals.metabolicCost / count;

        return statistics;
    }

} // namespace ae