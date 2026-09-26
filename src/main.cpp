#include <iomanip>
#include <iostream>
#include <vector>

#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/genetics/Genome.hpp"
#include "alien_evolution/simulation/Simulation.hpp"

namespace
{

    void printStatistics(
        const ae::GenerationStatistics& stats
    )
    {
        const double boundaryMaterialRatio =
            stats.meanMaterial > 0.0
            ? stats.meanBoundary / stats.meanMaterial
            : 0.0;

        std::cout
            << std::setw(4) << stats.generation
            << "  "
            << std::setw(3) << stats.populationSize
            << "  "
            << std::setw(10) << stats.meanFitness
            << "  "
            << std::setw(10) << stats.maximumFitness
            << "  "
            << std::setw(10) << stats.meanMaterial
            << "  "
            << std::setw(10) << stats.meanBoundary
            << "  "
            << std::setw(10) << boundaryMaterialRatio
            << "  "
            << std::setw(8) << stats.meanGenome.alphaR
            << "  "
            << std::setw(8) << stats.meanGenome.alphaE
            << "  "
            << std::setw(8) << stats.meanGenome.theta
            << "  "
            << std::setw(8) << stats.meanGenome.lambda
            << "  "
            << std::setw(8) << stats.meanGenome.beta
            << "  "
            << std::setw(8) << stats.meanGenome.growthRate
            << "  "
            << std::setw(8) << stats.meanGenome.metabolicCost
            << '\n';
    }

} // namespace

int main()
{
    // ------------------------------------------------------------
    // Environment
    // ------------------------------------------------------------

    ae::Environment environment{};

    environment.gravity = 9.80665;
    environment.resourceAvailability = 1.0;

    // ------------------------------------------------------------
    // Founder genome
    // ------------------------------------------------------------

    ae::Genome founder{};

    // ------------------------------------------------------------
    // Simulation configuration
    // ------------------------------------------------------------

    ae::SimulationConfig config{};

    config.populationSize = 100;

    config.developmentWidth = 40;
    config.developmentHeight = 30;
    config.developmentSteps = 10;

    // Initial population begins with variation around the founder.
    config.initialVariation.probabilityPerParameter = 1.0;
    config.initialVariation.logStandardDeviation = 0.10;

    // Subsequent offspring mutate more conservatively.
    config.offspringMutation.probabilityPerParameter = 0.10;
    config.offspringMutation.logStandardDeviation = 0.05;

    config.energetics.resourceGainPerBoundary = 1.0;
    config.energetics.maintenanceCostPerMaterial = 0.10;

    // ------------------------------------------------------------
    // Experiment
    // ------------------------------------------------------------

    constexpr std::uint64_t seed = 12345;
    constexpr std::size_t generations = 100;

    ae::Simulation simulation(
        environment,
        founder,
        config,
        seed
    );

    std::cout << std::fixed << std::setprecision(4);

    std::cout
        << "AlienEvolution v0.1.0\n"
        << "First evolutionary experiment\n\n";

    std::cout
        << "Seed: " << seed << '\n'
        << "Population: " << config.populationSize << '\n'
        << "Generations: " << generations << "\n\n";

    const std::vector<ae::GenerationStatistics> history =
        simulation.run(generations);

    std::cout
        << " Gen  Pop"
        << "    MeanFit"
        << "      MaxFit"
        << "    Material"
        << "    Boundary"
        << "       B/M"
        << "    alphaR"
        << "    alphaE"
        << "     theta"
        << "    lambda"
        << "      beta"
        << "    growth"
        << "      cost"
        << '\n';

    std::cout
        << "-------------------------------------------------------------"
        << "-------------------------------------------------------------"
        << "--------------------------------\n";

    for (std::size_t i = 0; i < history.size(); ++i)
    {
        if (
            i == 0
            || i % 10 == 0
            || i + 1 == history.size()
            )
        {
            printStatistics(history[i]);
        }
    }

    if (simulation.extinct())
    {
        std::cout
            << "\nPopulation went extinct.\n";
    }
    else
    {
        std::cout
            << "\nPopulation survived "
            << history.size()
            << " evaluated generations.\n";
    }

    return 0;
}