#include <cmath>
#include <iostream>
#include <stdexcept>

#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/genetics/Genome.hpp"
#include "alien_evolution/simulation/Simulation.hpp"

namespace
{

    void require(
        const bool condition,
        const char* message
    )
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    bool nearlyEqual(
        const double a,
        const double b,
        const double tolerance = 1.0e-12
    )
    {
        return std::abs(a - b) <= tolerance;
    }

    bool statisticsEqual(
        const ae::GenerationStatistics& a,
        const ae::GenerationStatistics& b
    )
    {
        return
            a.generation == b.generation
            && a.populationSize == b.populationSize
            && nearlyEqual(a.meanFitness, b.meanFitness)
            && nearlyEqual(a.maximumFitness, b.maximumFitness)
            && nearlyEqual(a.meanMaterial, b.meanMaterial)
            && nearlyEqual(a.meanBoundary, b.meanBoundary)
            && nearlyEqual(a.meanNetEnergy, b.meanNetEnergy)
            && nearlyEqual(
                a.meanGenome.alphaR,
                b.meanGenome.alphaR
            )
            && nearlyEqual(
                a.meanGenome.alphaE,
                b.meanGenome.alphaE
            )
            && nearlyEqual(
                a.meanGenome.theta,
                b.meanGenome.theta
            )
            && nearlyEqual(
                a.meanGenome.lambda,
                b.meanGenome.lambda
            )
            && nearlyEqual(
                a.meanGenome.beta,
                b.meanGenome.beta
            )
            && nearlyEqual(
                a.meanGenome.growthRate,
                b.meanGenome.growthRate
            )
            && nearlyEqual(
                a.meanGenome.metabolicCost,
                b.meanGenome.metabolicCost
            );
    }

} // namespace

int main()
{
    try
    {
        const ae::Genome founder{};
        const ae::Environment environment{};

        ae::SimulationConfig config{};

        config.populationSize = 40;

        config.developmentWidth = 20;
        config.developmentHeight = 20;
        config.developmentSteps = 6;

        config.initialVariation.probabilityPerParameter = 1.0;
        config.initialVariation.logStandardDeviation = 0.05;

        config.offspringMutation.probabilityPerParameter = 0.10;
        config.offspringMutation.logStandardDeviation = 0.05;

        // --------------------------------------------------------
        // Test 1:
        // One generation evaluates organisms and advances time.
        // --------------------------------------------------------

        ae::Simulation simulation(
            environment,
            founder,
            config,
            12345
        );

        const ae::GenerationStatistics first =
            simulation.step();

        require(
            first.generation == 0,
            "First evaluated generation should be generation zero."
        );

        require(
            first.populationSize == config.populationSize,
            "Simulation evaluated incorrect population size."
        );

        require(
            first.meanFitness > 0.0,
            "Reference population produced no viable fitness."
        );

        require(
            simulation.generation() == 1,
            "Simulation generation failed to advance."
        );

        require(
            simulation.population().size()
            == config.populationSize,
            "Viable population failed to reproduce to target size."
        );

        // New offspring should not yet have been evaluated.
        for (const ae::Organism& organism :
            simulation.population().organisms())
        {
            require(
                !organism.hasPhenotype(),
                "New generation unexpectedly retained phenotype."
            );

            require(
                !organism.hasFitness(),
                "New generation unexpectedly retained fitness."
            );
        }

        // --------------------------------------------------------
        // Test 2:
        // Identical seeds reproduce identical evolutionary history.
        // --------------------------------------------------------

        ae::Simulation simulationA(
            environment,
            founder,
            config,
            987654321
        );

        ae::Simulation simulationB(
            environment,
            founder,
            config,
            987654321
        );

        const auto historyA =
            simulationA.run(10);

        const auto historyB =
            simulationB.run(10);

        require(
            historyA.size() == historyB.size(),
            "Identical runs produced different history lengths."
        );

        for (std::size_t i = 0;
            i < historyA.size();
            ++i)
        {
            require(
                statisticsEqual(
                    historyA[i],
                    historyB[i]
                ),
                "Identical seeds produced different evolutionary histories."
            );
        }

        // --------------------------------------------------------
        // Test 3:
        // With no mutation or initial variation, the genome remains
        // unchanged through reproduction.
        // --------------------------------------------------------

        ae::SimulationConfig clonalConfig =
            config;

        clonalConfig.initialVariation.probabilityPerParameter = 0.0;
        clonalConfig.offspringMutation.probabilityPerParameter = 0.0;

        ae::Simulation clonalSimulation(
            environment,
            founder,
            clonalConfig,
            111
        );

        clonalSimulation.run(5);

        for (const ae::Organism& organism :
            clonalSimulation.population().organisms())
        {
            require(
                nearlyEqual(
                    organism.genome().alphaR,
                    founder.alphaR
                )
                && nearlyEqual(
                    organism.genome().alphaE,
                    founder.alphaE
                )
                && nearlyEqual(
                    organism.genome().theta,
                    founder.theta
                )
                && nearlyEqual(
                    organism.genome().lambda,
                    founder.lambda
                )
                && nearlyEqual(
                    organism.genome().beta,
                    founder.beta
                )
                && nearlyEqual(
                    organism.genome().growthRate,
                    founder.growthRate
                )
                && nearlyEqual(
                    organism.genome().metabolicCost,
                    founder.metabolicCost
                ),
                "Genome changed despite mutation being disabled."
            );
        }

        // --------------------------------------------------------
        // Test 4:
        // A resource-free environment causes extinction.
        // --------------------------------------------------------

        ae::Environment sterileEnvironment{};
        sterileEnvironment.resourceAvailability = 0.0;

        ae::Simulation extinctSimulation(
            sterileEnvironment,
            founder,
            config,
            222
        );

        const ae::GenerationStatistics extinctStats =
            extinctSimulation.step();

        require(
            nearlyEqual(
                extinctStats.meanFitness,
                0.0
            ),
            "Resource-free environment produced positive fitness."
        );

        require(
            extinctSimulation.extinct(),
            "Zero-fitness population failed to go extinct."
        );

        std::cout
            << "All simulation tests passed.\n";

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr
            << "TEST FAILURE: "
            << error.what()
            << '\n';

        return 1;
    }
}