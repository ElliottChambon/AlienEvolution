#include <array>
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

    constexpr std::size_t REPLICATES = 20;
    constexpr std::size_t GENERATIONS = 500;
    constexpr std::uint64_t BASE_SEED = 300000;

    constexpr std::array<std::size_t, 7> CHECKPOINTS{
        0,
        50,
        100,
        200,
        300,
        400,
        499
    };

    struct Treatment
    {
        std::string name;

        double mutationProbability = 0.0;
        double mutationSigma = 0.0;
    };

    struct CheckpointResult
    {
        std::size_t generation = 0;

        double meanFitness = 0.0;
        double maximumFitness = 0.0;

        double meanMaterial = 0.0;
        double meanBoundary = 0.0;
        double boundaryMaterialRatio = 0.0;

        double meanAlphaR = 0.0;
        double meanAlphaE = 0.0;
        double meanTheta = 0.0;
        double meanLambda = 0.0;
        double meanBeta = 0.0;
        double meanGrowthRate = 0.0;
        double meanMetabolicCost = 0.0;
    };

    struct RunResult
    {
        std::uint64_t seed = 0;

        std::vector<CheckpointResult> checkpoints;

        bool extinct = false;
    };

    struct TreatmentResult
    {
        Treatment treatment{};
        std::vector<RunResult> runs;
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

        return total
            / static_cast<double>(
                values.size()
                );
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

        // All treatments begin with the same standing variation.
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
        return
        {
            {
                "No new mutation",
                0.0,
                0.0
            },
            {
                "Gentle mutation",
                0.03,
                0.01
            },
            {
                "Moderate-high mutation",
                0.10,
                0.10
            },
            {
                "High probability moderate step",
                0.30,
                0.05
            },
            {
                "High probability large step",
                0.30,
                0.10
            }
        };
    }

    CheckpointResult makeCheckpoint(
        const ae::GenerationStatistics& statistics
    )
    {
        CheckpointResult checkpoint{};

        checkpoint.generation =
            statistics.generation;

        checkpoint.meanFitness =
            statistics.meanFitness;

        checkpoint.maximumFitness =
            statistics.maximumFitness;

        checkpoint.meanMaterial =
            statistics.meanMaterial;

        checkpoint.meanBoundary =
            statistics.meanBoundary;

        checkpoint.boundaryMaterialRatio =
            boundaryMaterialRatio(statistics);

        checkpoint.meanAlphaR =
            statistics.meanGenome.alphaR;

        checkpoint.meanAlphaE =
            statistics.meanGenome.alphaE;

        checkpoint.meanTheta =
            statistics.meanGenome.theta;

        checkpoint.meanLambda =
            statistics.meanGenome.lambda;

        checkpoint.meanBeta =
            statistics.meanGenome.beta;

        checkpoint.meanGrowthRate =
            statistics.meanGenome.growthRate;

        checkpoint.meanMetabolicCost =
            statistics.meanGenome.metabolicCost;

        return checkpoint;
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
            << treatment.name << ','
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
        std::ofstream& historyCsv
    )
    {
        const ae::Environment environment{};
        const ae::Genome founder{};

        TreatmentResult result{};
        result.treatment = treatment;
        result.runs.reserve(REPLICATES);

        std::cout
            << "\nRunning: "
            << treatment.name
            << "\n";

        std::cout
            << "  p = "
            << treatment.mutationProbability
            << ", sigma = "
            << treatment.mutationSigma
            << "\n";

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
                    "Simulation produced no history."
                );
            }

            RunResult run{};
            run.seed = seed;
            run.extinct = simulation.extinct();

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

                for (const std::size_t checkpointGeneration :
                CHECKPOINTS)
                {
                    if (
                        statistics.generation
                        == checkpointGeneration
                        )
                    {
                        run.checkpoints.push_back(
                            makeCheckpoint(
                                statistics
                            )
                        );
                    }
                }
            }

            if (
                !run.extinct
                && run.checkpoints.size()
                != CHECKPOINTS.size()
                )
            {
                throw std::runtime_error(
                    "A surviving run did not contain all expected checkpoints."
                );
            }

            result.runs.push_back(
                run
            );

            std::cout
                << "  replicate "
                << std::setw(2)
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

        const TreatmentResult& baseline =
            results.front();

        for (std::size_t treatmentIndex = 1;
            treatmentIndex < results.size();
            ++treatmentIndex)
        {
            const TreatmentResult& treatment =
                results[treatmentIndex];

            if (
                treatment.runs.size()
                != baseline.runs.size()
                )
            {
                throw std::runtime_error(
                    "Treatment replicate counts do not match."
                );
            }

            for (std::size_t replicate = 0;
                replicate < baseline.runs.size();
                ++replicate)
            {
                if (
                    baseline.runs[replicate].checkpoints.empty()
                    || treatment.runs[replicate].checkpoints.empty()
                    )
                {
                    throw std::runtime_error(
                        "Missing generation-zero checkpoint."
                    );
                }

                const double baselineFitness =
                    baseline.runs[replicate]
                    .checkpoints.front()
                    .meanFitness;

                const double treatmentFitness =
                    treatment.runs[replicate]
                    .checkpoints.front()
                    .meanFitness;

                if (
                    std::abs(
                        baselineFitness
                        - treatmentFitness
                    ) > 1.0e-12
                    )
                {
                    throw std::runtime_error(
                        "Matched treatments did not start from identical populations."
                    );
                }
            }
        }
    }

    void printCheckpointTable(
        const std::vector<TreatmentResult>& results
    )
    {
        std::cout
            << "\n\nMean fitness through evolutionary time\n\n";

        std::cout
            << std::left
            << std::setw(34)
            << "Treatment";

        for (const std::size_t checkpoint :
        CHECKPOINTS)
        {
            std::cout
                << std::right
                << std::setw(11)
                << checkpoint;
        }

        std::cout << '\n';

        std::cout
            << std::string(
                34 + CHECKPOINTS.size() * 11,
                '-'
            )
            << '\n';

        for (const TreatmentResult& treatment :
            results)
        {
            std::cout
                << std::left
                << std::setw(34)
                << treatment.treatment.name;

            for (std::size_t checkpointIndex = 0;
                checkpointIndex < CHECKPOINTS.size();
                ++checkpointIndex)
            {
                std::vector<double> fitnessValues;

                for (const RunResult& run :
                    treatment.runs)
                {
                    if (
                        checkpointIndex
                        < run.checkpoints.size()
                        )
                    {
                        fitnessValues.push_back(
                            run.checkpoints[
                                checkpointIndex
                            ].meanFitness
                        );
                    }
                }

                std::cout
                    << std::right
                    << std::setw(11)
                    << mean(fitnessValues);
            }

            std::cout << '\n';
        }
    }

    void printBoundaryMaterialTable(
        const std::vector<TreatmentResult>& results
    )
    {
        std::cout
            << "\n\nMean boundary/material ratio through time\n\n";

        std::cout
            << std::left
            << std::setw(34)
            << "Treatment";

        for (const std::size_t checkpoint :
        CHECKPOINTS)
        {
            std::cout
                << std::right
                << std::setw(11)
                << checkpoint;
        }

        std::cout << '\n';

        std::cout
            << std::string(
                34 + CHECKPOINTS.size() * 11,
                '-'
            )
            << '\n';

        for (const TreatmentResult& treatment :
            results)
        {
            std::cout
                << std::left
                << std::setw(34)
                << treatment.treatment.name;

            for (std::size_t checkpointIndex = 0;
                checkpointIndex < CHECKPOINTS.size();
                ++checkpointIndex)
            {
                std::vector<double> values;

                for (const RunResult& run :
                    treatment.runs)
                {
                    if (
                        checkpointIndex
                        < run.checkpoints.size()
                        )
                    {
                        values.push_back(
                            run.checkpoints[
                                checkpointIndex
                            ].boundaryMaterialRatio
                        );
                    }
                }

                std::cout
                    << std::right
                    << std::setw(11)
                    << mean(values);
            }

            std::cout << '\n';
        }
    }

    void printPairedDifferences(
        const std::vector<TreatmentResult>& results
    )
    {
        if (results.size() < 2)
        {
            return;
        }

        const TreatmentResult& baseline =
            results.front();

        std::cout
            << "\n\nMean paired fitness difference relative to no-new-mutation baseline\n\n";

        std::cout
            << std::left
            << std::setw(34)
            << "Treatment";

        for (const std::size_t checkpoint :
        CHECKPOINTS)
        {
            std::cout
                << std::right
                << std::setw(11)
                << checkpoint;
        }

        std::cout << '\n';

        std::cout
            << std::string(
                34 + CHECKPOINTS.size() * 11,
                '-'
            )
            << '\n';

        for (std::size_t treatmentIndex = 1;
            treatmentIndex < results.size();
            ++treatmentIndex)
        {
            const TreatmentResult& treatment =
                results[treatmentIndex];

            std::cout
                << std::left
                << std::setw(34)
                << treatment.treatment.name;

            for (std::size_t checkpointIndex = 0;
                checkpointIndex < CHECKPOINTS.size();
                ++checkpointIndex)
            {
                std::vector<double> differences;

                for (std::size_t replicate = 0;
                    replicate < baseline.runs.size();
                    ++replicate)
                {
                    if (
                        checkpointIndex
                        < baseline.runs[replicate]
                        .checkpoints.size()
                        && checkpointIndex
                        < treatment.runs[replicate]
                        .checkpoints.size()
                        )
                    {
                        differences.push_back(
                            treatment.runs[replicate]
                            .checkpoints[
                                checkpointIndex
                            ].meanFitness
                            -
                                    baseline.runs[replicate]
                                    .checkpoints[
                                        checkpointIndex
                                    ].meanFitness
                                    );
                    }
                }

                std::cout
                    << std::right
                    << std::setw(11)
                    << mean(differences);
            }

            std::cout << '\n';
        }
    }

    void printFinalStatistics(
        const std::vector<TreatmentResult>& results
    )
    {
        std::cout
            << "\n\nFinal-generation statistics\n\n";

        std::cout
            << std::left
            << std::setw(34)
            << "Treatment"

            << std::right
            << std::setw(12)
            << "Mean fit"

            << std::setw(12)
            << "SD fit"

            << std::setw(12)
            << "Mean B/M"

            << std::setw(10)
            << "Extinct"

            << '\n';

        std::cout
            << std::string(80, '-')
            << '\n';

        for (const TreatmentResult& treatment :
            results)
        {
            std::vector<double> fitnessValues;
            std::vector<double> boundaryMaterialValues;

            std::size_t extinctions = 0;

            for (const RunResult& run :
                treatment.runs)
            {
                if (run.extinct)
                {
                    ++extinctions;
                }

                if (!run.checkpoints.empty())
                {
                    const CheckpointResult & final =
                        run.checkpoints.back();

                    fitnessValues.push_back(
                        final.meanFitness
                    );

                    boundaryMaterialValues.push_back(
                        final.boundaryMaterialRatio
                    );
                }
            }

            std::cout
                << std::left
                << std::setw(34)
                << treatment.treatment.name

                << std::right
                << std::setw(12)
                << mean(fitnessValues)

                << std::setw(12)
                << sampleStandardDeviation(
                    fitnessValues
                )

                << std::setw(12)
                << mean(
                    boundaryMaterialValues
                )

                << std::setw(10)
                << extinctions

                << '\n';
        }
    }

    void writeCheckpointCsv(
        const std::vector<TreatmentResult>& results
    )
    {
        std::ofstream csv(
            "experiments/output/long_term_mutation_checkpoints.csv"
        );

        if (!csv)
        {
            throw std::runtime_error(
                "Could not create checkpoint CSV."
            );
        }

        csv << std::setprecision(17);

        csv
            << "treatment,"
            << "mutation_probability,"
            << "mutation_sigma,"
            << "replicate,"
            << "seed,"
            << "generation,"
            << "mean_fitness,"
            << "maximum_fitness,"
            << "mean_material,"
            << "mean_boundary,"
            << "boundary_material_ratio,"
            << "mean_alphaR,"
            << "mean_alphaE,"
            << "mean_theta,"
            << "mean_lambda,"
            << "mean_beta,"
            << "mean_growth_rate,"
            << "mean_metabolic_cost,"
            << "extinct"
            << '\n';

        for (const TreatmentResult& treatment :
            results)
        {
            for (std::size_t replicate = 0;
                replicate < treatment.runs.size();
                ++replicate)
            {
                const RunResult& run =
                    treatment.runs[replicate];

                for (const CheckpointResult& checkpoint :
                    run.checkpoints)
                {
                    csv
                        << treatment.treatment.name << ','
                        << treatment.treatment.mutationProbability << ','
                        << treatment.treatment.mutationSigma << ','
                        << replicate << ','
                        << run.seed << ','
                        << checkpoint.generation << ','
                        << checkpoint.meanFitness << ','
                        << checkpoint.maximumFitness << ','
                        << checkpoint.meanMaterial << ','
                        << checkpoint.meanBoundary << ','
                        << checkpoint.boundaryMaterialRatio << ','
                        << checkpoint.meanAlphaR << ','
                        << checkpoint.meanAlphaE << ','
                        << checkpoint.meanTheta << ','
                        << checkpoint.meanLambda << ','
                        << checkpoint.meanBeta << ','
                        << checkpoint.meanGrowthRate << ','
                        << checkpoint.meanMetabolicCost << ','
                        << (run.extinct ? 1 : 0)
                        << '\n';
                }
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

        std::ofstream historyCsv(
            "experiments/output/long_term_mutation_history.csv"
        );

        if (!historyCsv)
        {
            throw std::runtime_error(
                "Could not create history CSV."
            );
        }

        historyCsv << std::setprecision(17);

        historyCsv
            << "treatment,"
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

        const std::vector<Treatment> treatments =
            makeTreatments();

        std::vector<TreatmentResult> results;
        results.reserve(
            treatments.size()
        );

        std::cout
            << std::fixed
            << std::setprecision(4);

        std::cout
            << "AlienEvolution long-term mutation experiment\n\n";

        std::cout
            << "Treatments: "
            << treatments.size()
            << '\n';

        std::cout
            << "Replicates per treatment: "
            << REPLICATES
            << '\n';

        std::cout
            << "Generations: "
            << GENERATIONS
            << '\n';

        std::cout
            << "Population size: 100\n\n";

        std::cout
            << "Total evolutionary histories: "
            << treatments.size() * REPLICATES
            << '\n';

        std::cout
            << "Total evaluated generations: "
            << treatments.size()
            * REPLICATES
            * GENERATIONS
            << '\n';

        for (const Treatment& treatment :
            treatments)
        {
            results.push_back(
                runTreatment(
                    treatment,
                    historyCsv
                )
            );
        }

        verifyMatchedInitialConditions(
            results
        );

        writeCheckpointCsv(
            results
        );

        printCheckpointTable(
            results
        );

        printBoundaryMaterialTable(
            results
        );

        printPairedDifferences(
            results
        );

        printFinalStatistics(
            results
        );

        std::cout
            << "\nFull histories:\n"
            << "experiments/output/long_term_mutation_history.csv\n\n";

        std::cout
            << "Checkpoint data:\n"
            << "experiments/output/long_term_mutation_checkpoints.csv\n";

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