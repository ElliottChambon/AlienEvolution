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
#include "alien_evolution/genetics/Genome.hpp"
#include "alien_evolution/simulation/Simulation.hpp"

namespace
{

    constexpr std::size_t REPLICATES = 10;
    constexpr std::size_t GENERATIONS = 100;
    constexpr std::uint64_t BASE_SEED = 200000;

    struct Treatment
    {
        double mutationProbability = 0.0;
        double mutationSigma = 0.0;
    };

    struct RunSummary
    {
        std::uint64_t seed = 0;

        double initialFitness = 0.0;
        double finalFitness = 0.0;
        double fitnessChange = 0.0;

        double finalMaterial = 0.0;
        double finalBoundary = 0.0;
        double finalBoundaryMaterialRatio = 0.0;

        std::size_t finalGeneration = 0;

        bool extinct = false;
    };

    struct TreatmentResult
    {
        Treatment treatment{};
        std::vector<RunSummary> runs;
    };

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

    ae::SimulationConfig makeBaseConfig()
    {
        ae::SimulationConfig config{};

        config.populationSize = 100;

        config.developmentWidth = 40;
        config.developmentHeight = 30;
        config.developmentSteps = 10;

        // Every treatment begins from the same amount of
        // standing variation.
        config.initialVariation.probabilityPerParameter = 1.0;
        config.initialVariation.logStandardDeviation = 0.10;

        config.selectionMode =
            ae::SelectionMode::FitnessProportional;

        config.energetics.resourceGainPerBoundary = 1.0;
        config.energetics.maintenanceCostPerMaterial = 0.10;

        return config;
    }

    std::vector<Treatment> makeTreatments()
    {
        std::vector<Treatment> treatments;

        // No new mutations after generation zero.
        treatments.push_back(
            Treatment{
                0.0,
                0.0
            }
        );

        const std::vector<double> probabilities{
            0.01,
            0.03,
            0.10,
            0.30
        };

        const std::vector<double> sigmas{
            0.01,
            0.03,
            0.05,
            0.10
        };

        for (const double probability :
        probabilities)
        {
            for (const double sigma :
            sigmas)
            {
                treatments.push_back(
                    Treatment{
                        probability,
                        sigma
                    }
                );
            }
        }

        return treatments;
    }

    void writeHistoryRow(
        std::ofstream& csv,
        const Treatment& treatment,
        const std::size_t replicate,
        const std::uint64_t seed,
        const ae::GenerationStatistics& statistics
    )
    {
        csv
            << treatment.mutationProbability << ','
            << treatment.mutationSigma << ','
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

    TreatmentResult runTreatment(
        const Treatment& treatment,
        std::ofstream& historyCsv,
        std::ofstream& runCsv
    )
    {
        const ae::Environment environment{};
        const ae::Genome founder{};

        TreatmentResult result{};
        result.treatment = treatment;
        result.runs.reserve(REPLICATES);

        std::cout
            << "\nMutation probability = "
            << treatment.mutationProbability
            << ", sigma = "
            << treatment.mutationSigma
            << '\n';

        for (std::size_t replicate = 0;
            replicate < REPLICATES;
            ++replicate)
        {
            ae::SimulationConfig config =
                makeBaseConfig();

            config.offspringMutation.probabilityPerParameter =
                treatment.mutationProbability;

            config.offspringMutation.logStandardDeviation =
                treatment.mutationSigma;

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
                writeHistoryRow(
                    historyCsv,
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

            summary.seed =
                seed;

            summary.initialFitness =
                initial.meanFitness;

            summary.finalFitness =
                final.meanFitness;

            summary.fitnessChange =
                final.meanFitness
                - initial.meanFitness;

            summary.finalMaterial =
                final.meanMaterial;

            summary.finalBoundary =
                final.meanBoundary;

            summary.finalBoundaryMaterialRatio =
                boundaryMaterialRatio(final);

            summary.finalGeneration =
                final.generation;

            summary.extinct =
                simulation.extinct();

            result.runs.push_back(
                summary
            );

            runCsv
                << treatment.mutationProbability << ','
                << treatment.mutationSigma << ','
                << replicate << ','
                << seed << ','
                << summary.initialFitness << ','
                << summary.finalFitness << ','
                << summary.fitnessChange << ','
                << summary.finalMaterial << ','
                << summary.finalBoundary << ','
                << summary.finalBoundaryMaterialRatio << ','
                << summary.finalGeneration << ','
                << (summary.extinct ? 1 : 0)
                << '\n';

            std::cout
                << "  replicate "
                << replicate + 1
                << "/"
                << REPLICATES
                << " complete\n";
        }

        return result;
    }

    void verifyMatchedInitialConditions(
        const std::vector<TreatmentResult>& results
    )
    {
        if (results.empty())
        {
            return;
        }

        const std::vector<RunSummary>& baseline =
            results.front().runs;

        for (std::size_t treatmentIndex = 1;
            treatmentIndex < results.size();
            ++treatmentIndex)
        {
            const std::vector<RunSummary>& comparison =
                results[treatmentIndex].runs;

            if (comparison.size() != baseline.size())
            {
                throw std::runtime_error(
                    "Treatment replicate counts differ."
                );
            }

            for (std::size_t replicate = 0;
                replicate < baseline.size();
                ++replicate)
            {
                const double difference =
                    std::abs(
                        baseline[replicate].initialFitness
                        - comparison[replicate].initialFitness
                    );

                if (difference > 1.0e-12)
                {
                    throw std::runtime_error(
                        "Mutation treatments did not begin from matched initial populations."
                    );
                }
            }
        }
    }

    void printAggregateResult(
        const TreatmentResult& result
    )
    {
        std::vector<double> initialFitness;
        std::vector<double> finalFitness;
        std::vector<double> changes;
        std::vector<double> boundaryMaterialRatios;

        std::size_t extinctionCount = 0;

        for (const RunSummary& run :
            result.runs)
        {
            initialFitness.push_back(
                run.initialFitness
            );

            finalFitness.push_back(
                run.finalFitness
            );

            changes.push_back(
                run.fitnessChange
            );

            boundaryMaterialRatios.push_back(
                run.finalBoundaryMaterialRatio
            );

            if (run.extinct)
            {
                ++extinctionCount;
            }
        }

        std::cout
            << std::setw(8)
            << result.treatment.mutationProbability

            << std::setw(10)
            << result.treatment.mutationSigma

            << std::setw(12)
            << mean(initialFitness)

            << std::setw(12)
            << mean(finalFitness)

            << std::setw(12)
            << mean(changes)

            << std::setw(12)
            << sampleStandardDeviation(changes)

            << std::setw(12)
            << mean(boundaryMaterialRatios)

            << std::setw(8)
            << extinctionCount

            << '\n';
    }

    void printPairedComparisonToBaseline(
        const TreatmentResult& baseline,
        const TreatmentResult& treatment
    )
    {
        if (
            baseline.runs.size()
            != treatment.runs.size()
            )
        {
            throw std::runtime_error(
                "Cannot perform paired comparison with unequal replicate counts."
            );
        }

        std::vector<double> differences;

        std::size_t treatmentHigher = 0;
        std::size_t baselineHigher = 0;
        std::size_t equal = 0;

        for (std::size_t i = 0;
            i < baseline.runs.size();
            ++i)
        {
            const double difference =
                treatment.runs[i].finalFitness
                - baseline.runs[i].finalFitness;

            differences.push_back(
                difference
            );

            if (difference > 1.0e-12)
            {
                ++treatmentHigher;
            }
            else if (difference < -1.0e-12)
            {
                ++baselineHigher;
            }
            else
            {
                ++equal;
            }
        }

        std::cout
            << std::setw(8)
            << treatment.treatment.mutationProbability

            << std::setw(10)
            << treatment.treatment.mutationSigma

            << std::setw(16)
            << mean(differences)

            << std::setw(14)
            << sampleStandardDeviation(differences)

            << std::setw(10)
            << treatmentHigher

            << std::setw(10)
            << baselineHigher

            << std::setw(8)
            << equal

            << '\n';
    }

} // namespace

int main()
{
    try
    {
        std::filesystem::create_directories(
            "experiments/output"
        );

        std::ofstream historyCsv(
            "experiments/output/mutation_sweep_history.csv"
        );

        std::ofstream runCsv(
            "experiments/output/mutation_sweep_runs.csv"
        );

        if (!historyCsv || !runCsv)
        {
            throw std::runtime_error(
                "Could not create mutation sweep output files."
            );
        }

        historyCsv << std::setprecision(17);
        runCsv << std::setprecision(17);

        historyCsv
            << "mutation_probability,"
            << "mutation_sigma,"
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

        runCsv
            << "mutation_probability,"
            << "mutation_sigma,"
            << "replicate,"
            << "seed,"
            << "initial_fitness,"
            << "final_fitness,"
            << "fitness_change,"
            << "final_material,"
            << "final_boundary,"
            << "final_boundary_material_ratio,"
            << "final_generation,"
            << "extinct"
            << '\n';

        const std::vector<Treatment> treatments =
            makeTreatments();

        std::vector<TreatmentResult> results;
        results.reserve(
            treatments.size()
        );

        std::cout
            << "AlienEvolution mutation sweep\n\n";

        std::cout
            << "Treatments: "
            << treatments.size()
            << '\n';

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

        for (const Treatment& treatment :
            treatments)
        {
            results.push_back(
                runTreatment(
                    treatment,
                    historyCsv,
                    runCsv
                )
            );
        }

        verifyMatchedInitialConditions(
            results
        );

        std::cout
            << "\n\nAggregate mutation sweep\n\n";

        std::cout
            << std::setw(8)
            << "Prob"

            << std::setw(10)
            << "Sigma"

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
            << std::string(86, '-')
            << '\n';

        for (const TreatmentResult& result :
            results)
        {
            printAggregateResult(
                result
            );
        }

        const TreatmentResult& baseline =
            results.front();

        std::cout
            << "\n\nPaired comparison against no-new-mutation baseline\n\n";

        std::cout
            << std::setw(8)
            << "Prob"

            << std::setw(10)
            << "Sigma"

            << std::setw(16)
            << "Mean diff"

            << std::setw(14)
            << "SD diff"

            << std::setw(10)
            << "Mut > 0"

            << std::setw(10)
            << "Base >"

            << std::setw(8)
            << "Equal"

            << '\n';

        std::cout
            << std::string(76, '-')
            << '\n';

        for (std::size_t i = 1;
            i < results.size();
            ++i)
        {
            printPairedComparisonToBaseline(
                baseline,
                results[i]
            );
        }

        std::cout
            << "\nFull generation histories written to:\n"
            << "experiments/output/mutation_sweep_history.csv\n\n";

        std::cout
            << "Per-run summaries written to:\n"
            << "experiments/output/mutation_sweep_runs.csv\n";

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