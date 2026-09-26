#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/evolution/Reproduction.hpp"
#include "alien_evolution/genetics/Genome.hpp"
#include "alien_evolution/simulation/Simulation.hpp"

namespace
{

    constexpr std::size_t REPLICATES = 20;
    constexpr std::size_t GENERATIONS = 100;
    constexpr std::uint64_t BASE_SEED = 100000;

    struct Treatment
    {
        std::string name;

        ae::SelectionMode selectionMode =
            ae::SelectionMode::FitnessProportional;

        bool offspringMutation = true;
    };

    struct RunSummary
    {
        std::uint64_t seed = 0;

        double initialFitness = 0.0;
        double finalFitness = 0.0;
        double fitnessChange = 0.0;

        double initialBoundaryMaterial = 0.0;
        double finalBoundaryMaterial = 0.0;

        bool extinct = false;
    };

    double boundaryMaterialRatio(
        const ae::GenerationStatistics& statistics
    )
    {
        if (statistics.meanMaterial <= 0.0)
        {
            return 0.0;
        }

        return
            statistics.meanBoundary
            / statistics.meanMaterial;
    }

    double mean(
        const std::vector<double>& values
    )
    {
        if (values.empty())
        {
            return 0.0;
        }

        double total = 0.0;

        for (const double value : values)
        {
            total += value;
        }

        return
            total
            / static_cast<double>(values.size());
    }

    double sampleStandardDeviation(
        const std::vector<double>& values
    )
    {
        if (values.size() < 2)
        {
            return 0.0;
        }

        const double average =
            mean(values);

        double squaredDifferenceSum = 0.0;

        for (const double value : values)
        {
            const double difference =
                value - average;

            squaredDifferenceSum +=
                difference * difference;
        }

        return std::sqrt(
            squaredDifferenceSum
            / static_cast<double>(
                values.size() - 1
                )
        );
    }

    ae::SimulationConfig makeBaseConfig()
    {
        ae::SimulationConfig config{};

        config.populationSize = 100;

        config.developmentWidth = 40;
        config.developmentHeight = 30;
        config.developmentSteps = 10;

        config.initialVariation.probabilityPerParameter = 1.0;
        config.initialVariation.logStandardDeviation = 0.10;

        config.offspringMutation.probabilityPerParameter = 0.10;
        config.offspringMutation.logStandardDeviation = 0.05;

        config.energetics.resourceGainPerBoundary = 1.0;
        config.energetics.maintenanceCostPerMaterial = 0.10;

        return config;
    }

    void writeCsvRow(
        std::ofstream& csv,
        const Treatment& treatment,
        const std::size_t replicate,
        const std::uint64_t seed,
        const ae::GenerationStatistics& statistics
    )
    {
        csv
            << treatment.name << ','
            << replicate << ','
            << seed << ','
            << statistics.generation << ','
            << statistics.populationSize << ','
            << statistics.meanFitness << ','
            << statistics.maximumFitness << ','
            << statistics.meanMaterial << ','
            << statistics.meanBoundary << ','
            << boundaryMaterialRatio(statistics) << ','
            << statistics.meanNetEnergy << ','
            << statistics.meanGenome.alphaR << ','
            << statistics.meanGenome.alphaE << ','
            << statistics.meanGenome.theta << ','
            << statistics.meanGenome.lambda << ','
            << statistics.meanGenome.beta << ','
            << statistics.meanGenome.growthRate << ','
            << statistics.meanGenome.metabolicCost
            << '\n';
    }

    std::vector<RunSummary> runTreatment(
        const Treatment& treatment,
        std::ofstream& csv
    )
    {
        const ae::Environment environment{};
        const ae::Genome founder{};

        std::vector<RunSummary> summaries;
        summaries.reserve(REPLICATES);

        std::cout
            << "\nRunning treatment: "
            << treatment.name
            << '\n';

        for (std::size_t replicate = 0;
            replicate < REPLICATES;
            ++replicate)
        {
            ae::SimulationConfig config =
                makeBaseConfig();

            config.selectionMode =
                treatment.selectionMode;

            if (!treatment.offspringMutation)
            {
                config.offspringMutation.probabilityPerParameter =
                    0.0;
            }

            const std::uint64_t seed =
                BASE_SEED + replicate;

            ae::Simulation simulation(
                environment,
                founder,
                config,
                seed
            );

            const std::vector<ae::GenerationStatistics> history =
                simulation.run(GENERATIONS);

            if (history.empty())
            {
                throw std::runtime_error(
                    "Simulation produced no generation statistics."
                );
            }

            for (const ae::GenerationStatistics& statistics :
                history)
            {
                writeCsvRow(
                    csv,
                    treatment,
                    replicate,
                    seed,
                    statistics
                );
            }

            const ae::GenerationStatistics& initial =
                history.front();

            const ae::GenerationStatistics & final =
                history.back();

            RunSummary summary{};

            summary.seed = seed;

            summary.initialFitness =
                initial.meanFitness;

            summary.finalFitness =
                final.meanFitness;

            summary.fitnessChange =
                final.meanFitness
                - initial.meanFitness;

            summary.initialBoundaryMaterial =
                boundaryMaterialRatio(initial);

            summary.finalBoundaryMaterial =
                boundaryMaterialRatio(final);

            summary.extinct =
                simulation.extinct();

            summaries.push_back(summary);

            std::cout
                << "  replicate "
                << std::setw(2)
                << replicate + 1
                << "/"
                << REPLICATES
                << " complete\n";
        }

        return summaries;
    }

    void printTreatmentSummary(
        const std::string& name,
        const std::vector<RunSummary>& runs
    )
    {
        std::vector<double> initialFitness;
        std::vector<double> finalFitness;
        std::vector<double> fitnessChange;
        std::vector<double> finalBoundaryMaterial;

        std::size_t extinctions = 0;

        for (const RunSummary& run : runs)
        {
            initialFitness.push_back(
                run.initialFitness
            );

            finalFitness.push_back(
                run.finalFitness
            );

            fitnessChange.push_back(
                run.fitnessChange
            );

            finalBoundaryMaterial.push_back(
                run.finalBoundaryMaterial
            );

            if (run.extinct)
            {
                ++extinctions;
            }
        }

        std::cout
            << std::left
            << std::setw(30)
            << name

            << std::right
            << std::setw(12)
            << mean(initialFitness)

            << std::setw(12)
            << mean(finalFitness)

            << std::setw(12)
            << mean(fitnessChange)

            << std::setw(12)
            << sampleStandardDeviation(fitnessChange)

            << std::setw(12)
            << mean(finalBoundaryMaterial)

            << std::setw(8)
            << extinctions

            << '\n';
    }

    void printPairedComparison(
        const std::string& nameA,
        const std::vector<RunSummary>& a,
        const std::string& nameB,
        const std::vector<RunSummary>& b
    )
    {
        if (a.size() != b.size())
        {
            throw std::runtime_error(
                "Paired treatments have different replicate counts."
            );
        }

        std::vector<double> differences;

        std::size_t aHigher = 0;
        std::size_t equal = 0;
        std::size_t bHigher = 0;

        for (std::size_t i = 0;
            i < a.size();
            ++i)
        {
            const double difference =
                a[i].finalFitness
                - b[i].finalFitness;

            differences.push_back(
                difference
            );

            if (difference > 1.0e-12)
            {
                ++aHigher;
            }
            else if (difference < -1.0e-12)
            {
                ++bHigher;
            }
            else
            {
                ++equal;
            }
        }

        std::cout
            << "\n"
            << nameA
            << " minus "
            << nameB
            << '\n';

        std::cout
            << "  Mean paired final-fitness difference: "
            << mean(differences)
            << '\n';

        std::cout
            << "  SD of paired differences: "
            << sampleStandardDeviation(differences)
            << '\n';

        std::cout
            << "  "
            << nameA
            << " higher: "
            << aHigher
            << "/"
            << a.size()
            << '\n';

        std::cout
            << "  Equal: "
            << equal
            << "/"
            << a.size()
            << '\n';

        std::cout
            << "  "
            << nameB
            << " higher: "
            << bHigher
            << "/"
            << a.size()
            << '\n';
    }

    void verifyMatchedInitialConditions(
        const std::vector<RunSummary>& a,
        const std::vector<RunSummary>& b,
        const std::vector<RunSummary>& c
    )
    {
        if (
            a.size() != b.size()
            || a.size() != c.size()
            )
        {
            throw std::runtime_error(
                "Treatment replicate counts do not match."
            );
        }

        for (std::size_t i = 0;
            i < a.size();
            ++i)
        {
            const double ab =
                std::abs(
                    a[i].initialFitness
                    - b[i].initialFitness
                );

            const double ac =
                std::abs(
                    a[i].initialFitness
                    - c[i].initialFitness
                );

            if (
                ab > 1.0e-12
                || ac > 1.0e-12
                )
            {
                throw std::runtime_error(
                    "Matched treatments did not begin from identical initial conditions."
                );
            }
        }
    }

} // namespace

int main()
{
    try
    {
        std::filesystem::create_directories(
            "experiments/output"
        );

        std::ofstream csv(
            "experiments/output/selection_validation.csv"
        );

        if (!csv)
        {
            throw std::runtime_error(
                "Could not open experiment CSV output file."
            );
        }

        csv << std::setprecision(17);

        csv
            << "treatment,"
            << "replicate,"
            << "seed,"
            << "generation,"
            << "population_size,"
            << "mean_fitness,"
            << "maximum_fitness,"
            << "mean_material,"
            << "mean_boundary,"
            << "boundary_material_ratio,"
            << "mean_net_energy,"
            << "mean_alphaR,"
            << "mean_alphaE,"
            << "mean_theta,"
            << "mean_lambda,"
            << "mean_beta,"
            << "mean_growth_rate,"
            << "mean_metabolic_cost"
            << '\n';

        const Treatment selectedMutation{
            "Selection + mutation",
            ae::SelectionMode::FitnessProportional,
            true
        };

        const Treatment neutralMutation{
            "Neutral + mutation",
            ae::SelectionMode::Uniform,
            true
        };

        const Treatment standingVariation{
            "Selection + standing variation",
            ae::SelectionMode::FitnessProportional,
            false
        };

        std::cout
            << "AlienEvolution selection validation\n\n";

        std::cout
            << "Replicates per treatment: "
            << REPLICATES
            << '\n';

        std::cout
            << "Generations per replicate: "
            << GENERATIONS
            << '\n';

        std::cout
            << "Population size: 100\n";

        const std::vector<RunSummary> selectedRuns =
            runTreatment(
                selectedMutation,
                csv
            );

        const std::vector<RunSummary> neutralRuns =
            runTreatment(
                neutralMutation,
                csv
            );

        const std::vector<RunSummary> standingRuns =
            runTreatment(
                standingVariation,
                csv
            );

        verifyMatchedInitialConditions(
            selectedRuns,
            neutralRuns,
            standingRuns
        );

        std::cout
            << "\n\nAggregate results\n\n";

        std::cout
            << std::left
            << std::setw(30)
            << "Treatment"

            << std::right
            << std::setw(12)
            << "Initial"

            << std::setw(12)
            << "Final"

            << std::setw(12)
            << "Change"

            << std::setw(12)
            << "SD change"

            << std::setw(12)
            << "Final B/M"

            << std::setw(8)
            << "Extinct"

            << '\n';

        std::cout
            << std::string(98, '-')
            << '\n';

        printTreatmentSummary(
            selectedMutation.name,
            selectedRuns
        );

        printTreatmentSummary(
            neutralMutation.name,
            neutralRuns
        );

        printTreatmentSummary(
            standingVariation.name,
            standingRuns
        );

        printPairedComparison(
            selectedMutation.name,
            selectedRuns,
            neutralMutation.name,
            neutralRuns
        );

        printPairedComparison(
            selectedMutation.name,
            selectedRuns,
            standingVariation.name,
            standingRuns
        );

        std::cout
            << "\nFull generation-by-generation data written to:\n"
            << "experiments/output/selection_validation.csv\n";

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr
            << "\nEXPERIMENT FAILURE: "
            << error.what()
            << '\n';

        return 1;
    }
}