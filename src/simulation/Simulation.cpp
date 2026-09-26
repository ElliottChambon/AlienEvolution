#include "alien_evolution/simulation/Simulation.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "alien_evolution/evaluation/Fitness.hpp"
#include "alien_evolution/evaluation/PhenotypeMetrics.hpp"
#include "alien_evolution/evolution/Reproduction.hpp"

namespace ae
{

    namespace
    {

        bool hasRequiredDevelopmentInterface(
            const RegulatoryProgram& program,
            const RegulatoryDevelopmentConfig& config
        )
        {
            return
                program.containsNode(
                    config.localMaterialInputNodeId
                )
                && program.containsNode(
                    config.resourceInputNodeId
                )
                && program.containsNode(
                    config.depositionOutputNodeId
                );
        }

        const RegulatoryProgram& validateFounderProgram(
            const RegulatoryProgram& program,
            const RegulatoryDevelopmentConfig& config
        )
        {
            if (
                !hasRequiredDevelopmentInterface(
                    program,
                    config
                )
                )
            {
                throw std::invalid_argument(
                    "Founder regulatory program does not contain all required developmental interface nodes."
                );
            }

            if (
                config.depositionOutputNodeId
                == config.localMaterialInputNodeId
                || config.depositionOutputNodeId
                == config.resourceInputNodeId
                )
            {
                throw std::invalid_argument(
                    "Founder developmental output node must be distinct from input nodes."
                );
            }

            return program;
        }

    } // namespace

    Simulation::Simulation(
        Environment environment,
        RegulatoryProgram founderProgram,
        SimulationConfig config,
        const std::uint64_t seed
    )
        : environment_(
            std::move(
                environment
            )
        ),
        config_(
            std::move(
                config
            )
        ),
        random_(
            seed
        ),
        development_(
            config_.developmentWidth,
            config_.developmentHeight,
            config_.developmentSteps,
            config_.development
        ),
        population_(
            config_.populationSize,
            validateFounderProgram(
                founderProgram,
                config_.development
            ),
            random_,
            config_.initialVariation
        )
    {}

    const Population&
        Simulation::population() const
    {
        return population_;
    }

    const Environment&
        Simulation::environment() const
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

    std::vector<GenerationStatistics>
        Simulation::run(
            const std::size_t generationCount
        )
    {
        std::vector<GenerationStatistics> history;

        history.reserve(
            generationCount
        );

        for (
            std::size_t i = 0;
            i < generationCount;
            ++i
            )
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

    GenerationStatistics
        Simulation::evaluatePopulation()
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

        double totalFitness =
            0.0;

        double maximumFitness =
            0.0;

        double totalMaterial =
            0.0;

        double totalBoundary =
            0.0;

        double totalNetEnergy =
            0.0;

        double totalNodeCount =
            0.0;

        double totalInteractionCount =
            0.0;

        double totalNetworkDensity =
            0.0;

        std::size_t developmentFailureCount =
            0;

        for (
            std::size_t i = 0;
            i < population_.size();
            ++i
            )
        {
            Organism& organism =
                population_.at(i);

            const RegulatoryProgram& program =
                organism.regulatoryProgram();

            Phenotype phenotype(
                config_.developmentWidth,
                config_.developmentHeight
            );

            if (
                hasRequiredDevelopmentInterface(
                    program,
                    config_.development
                )
                )
            {
                phenotype =
                    development_.develop(
                        program,
                        environment_
                    );
            }
            else
            {
                // Losing a required developmental interface through
                // mutation is treated as a nonviable developmental
                // outcome rather than as a program-wide exception.
                ++developmentFailureCount;
            }

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

            totalFitness +=
                fitness;

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

            const double nodeCount =
                static_cast<double>(
                    program.nodeCount()
                    );

            const double interactionCount =
                static_cast<double>(
                    program.interactionCount()
                    );

            totalNodeCount +=
                nodeCount;

            totalInteractionCount +=
                interactionCount;

            const double possibleInteractions =
                nodeCount * nodeCount;

            if (possibleInteractions > 0.0)
            {
                totalNetworkDensity +=
                    interactionCount
                    / possibleInteractions;
            }
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

        statistics.meanRegulatoryNodeCount =
            totalNodeCount / count;

        statistics.meanRegulatoryInteractionCount =
            totalInteractionCount / count;

        statistics.meanRegulatoryNetworkDensity =
            totalNetworkDensity / count;

        statistics.developmentFailureCount =
            developmentFailureCount;

        statistics.developmentFailureFraction =
            static_cast<double>(
                developmentFailureCount
                )
            / count;

        return statistics;
    }

} // namespace ae