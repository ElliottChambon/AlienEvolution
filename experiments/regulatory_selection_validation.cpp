#include <algorithm>
#include <chrono>
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
#include "alien_evolution/genetics/RegulatoryProgram.hpp"
#include "alien_evolution/simulation/Simulation.hpp"

namespace
{

    constexpr std::size_t replicateCount =
        50;

    constexpr std::size_t generationCount =
        100;

    constexpr std::size_t populationSize =
        20;

    constexpr std::uint64_t baseSeed =
        20260926ULL;


    // ============================================================
    // DATA STRUCTURES
    // ============================================================

    struct RunOutcome
    {
        double initialMeanFitness =
            0.0;

        double finalMeanFitness =
            0.0;

        double finalMaximumFitness =
            0.0;

        // Mean fitness among organisms that retained the required
        // developmental interface.
        double finalViableMeanFitness =
            0.0;

        double finalMeanMaterial =
            0.0;

        double finalMeanBoundary =
            0.0;

        double finalMeanNetEnergy =
            0.0;

        double finalMeanNodeCount =
            0.0;

        double finalMeanInteractionCount =
            0.0;

        double finalMeanNetworkDensity =
            0.0;

        double finalDevelopmentFailureFraction =
            0.0;

        double meanDevelopmentFailureFraction =
            0.0;

        std::size_t generationsObserved =
            0;

        bool extinct =
            false;
    };


    struct Summary
    {
        double mean =
            0.0;

        double standardDeviation =
            0.0;

        double median =
            0.0;

        double minimum =
            0.0;

        double maximum =
            0.0;
    };


    // ============================================================
    // FOUNDER
    // ============================================================

    ae::RegulatoryProgram makeFounderProgram()
    {
        return ae::RegulatoryProgram(
            {
                // Local-material environmental input.
                {
                    1,
                    0.0,
                    0.0,
                    1.0
                },

            // Resource environmental input.
            {
                2,
                0.0,
                0.0,
                1.0
            },

            // Material-deposition effector.
            {
                3,
                0.0,
                0.01,
                1.0
            }
            },
        {
            {
                1,
                3,
                20.0,
                0.20,
                2.0
            },

            {
                2,
                3,
                20.0,
                0.50,
                2.0
            }
        }
        );
    }


    // ============================================================
    // MUTATION MODELS
    // ============================================================

    // ------------------------------------------------------------
    // Ongoing full V0.2 mutation model.
    //
    // IMPORTANT:
    // These values are explicit experimental null-model settings.
    // They are NOT calibrated biological mutation rates.
    // ------------------------------------------------------------

    ae::RegulatoryMutationGeneratorConfig
        makeFullMutationConfig()
    {
        ae::RegulatoryMutationGeneratorConfig config{};

        config.rates
            .nodeKineticPerNode =
            0.02;

        config.rates
            .interactionParameterPerInteraction =
            0.02;

        config.rates
            .interactionGainPerAbsentPair =
            0.002;

        config.rates
            .interactionLossPerInteraction =
            0.002;

        config.rates
            .regulatoryUnitDuplicationPerNode =
            0.001;

        config.rates
            .regulatoryNodeLossPerDeletableNode =
            0.001;


        config.quantitativeEffects
            .nodeBasalProductionLogStdDev =
            0.05;

        config.quantitativeEffects
            .nodeDegradationLogStdDev =
            0.05;

        config.quantitativeEffects
            .interactionFoldChangeLogStdDev =
            0.05;

        config.quantitativeEffects
            .interactionHalfSaturationLogStdDev =
            0.05;

        config.quantitativeEffects
            .interactionCooperativityLogStdDev =
            0.05;


        config.interactionGain
            .foldChangeLogStdDev =
            0.25;

        config.interactionGain
            .referenceHalfSaturation =
            0.5;

        config.interactionGain
            .halfSaturationLogStdDev =
            0.10;

        config.interactionGain
            .referenceCooperativity =
            2.0;

        config.interactionGain
            .cooperativityLogStdDev =
            0.10;

        return config;
    }


    // ------------------------------------------------------------
    // Quantitative-only ongoing mutation.
    //
    // Network topology cannot change.
    // ------------------------------------------------------------

    ae::RegulatoryMutationGeneratorConfig
        makeQuantitativeMutationConfig()
    {
        ae::RegulatoryMutationGeneratorConfig config{};

        config.rates
            .nodeKineticPerNode =
            0.02;

        config.rates
            .interactionParameterPerInteraction =
            0.02;


        config.quantitativeEffects
            .nodeBasalProductionLogStdDev =
            0.05;

        config.quantitativeEffects
            .nodeDegradationLogStdDev =
            0.05;

        config.quantitativeEffects
            .interactionFoldChangeLogStdDev =
            0.05;

        config.quantitativeEffects
            .interactionHalfSaturationLogStdDev =
            0.05;

        config.quantitativeEffects
            .interactionCooperativityLogStdDev =
            0.05;

        return config;
    }


    // ------------------------------------------------------------
    // Initial standing variation.
    //
    // All five treatments receive this EXACT SAME process and seed.
    //
    // It deliberately permits only quantitative variation so every
    // treatment starts with the same 3-node / 2-edge topology.
    //
    // The higher rates here create useful standing variation at
    // generation zero. They are experimental initialization values,
    // not biological mutation-rate estimates.
    // ------------------------------------------------------------

    ae::RegulatoryMutationGeneratorConfig
        makeInitialStandingVariationConfig()
    {
        ae::RegulatoryMutationGeneratorConfig config{};

        config.rates
            .nodeKineticPerNode =
            0.20;

        config.rates
            .interactionParameterPerInteraction =
            0.20;


        config.quantitativeEffects
            .nodeBasalProductionLogStdDev =
            0.08;

        config.quantitativeEffects
            .nodeDegradationLogStdDev =
            0.08;

        config.quantitativeEffects
            .interactionFoldChangeLogStdDev =
            0.08;

        config.quantitativeEffects
            .interactionHalfSaturationLogStdDev =
            0.08;

        config.quantitativeEffects
            .interactionCooperativityLogStdDev =
            0.08;

        return config;
    }


    // ============================================================
    // SIMULATION CONFIG
    // ============================================================

    ae::SimulationConfig makeBaseSimulationConfig()
    {
        ae::SimulationConfig config{};

        config.populationSize =
            populationSize;

        config.developmentWidth =
            15;

        config.developmentHeight =
            15;

        config.developmentSteps =
            5;


        config.development
            .localMaterialInputNodeId =
            1;

        config.development
            .resourceInputNodeId =
            2;

        config.development
            .depositionOutputNodeId =
            3;

        config.development
            .neighborhoodLengthScale =
            1.0;

        config.development
            .regulatoryTimeStep =
            0.05;

        config.development
            .regulatoryStepsPerDevelopmentStep =
            6;

        config.development
            .outputHalfSaturation =
            0.2;

        config.development
            .outputCooperativity =
            2.0;

        config.development
            .depositionRateScale =
            0.25;


        config.initialVariation =
            makeInitialStandingVariationConfig();

        return config;
    }


    // ============================================================
    // STATISTICAL HELPERS
    // ============================================================

    Summary summarize(
        const std::vector<double>& values
    )
    {
        if (values.empty())
        {
            return {};
        }

        double total =
            0.0;

        for (const double value :
        values)
        {
            total +=
                value;
        }

        const double mean =
            total
            / static_cast<double>(
                values.size()
                );


        double squaredDifferenceTotal =
            0.0;

        for (const double value :
        values)
        {
            const double difference =
                value - mean;

            squaredDifferenceTotal +=
                difference * difference;
        }


        double standardDeviation =
            0.0;

        if (values.size() > 1)
        {
            standardDeviation =
                std::sqrt(
                    squaredDifferenceTotal
                    / static_cast<double>(
                        values.size() - 1
                        )
                );
        }


        std::vector<double> sorted =
            values;

        std::sort(
            sorted.begin(),
            sorted.end()
        );


        double median =
            0.0;

        if (
            sorted.size() % 2
            == 0
            )
        {
            const std::size_t upper =
                sorted.size() / 2;

            const std::size_t lower =
                upper - 1;

            median =
                (
                    sorted[lower]
                    + sorted[upper]
                    )
                / 2.0;
        }
        else
        {
            median =
                sorted[
                    sorted.size() / 2
                ];
        }


        return {
            mean,
            standardDeviation,
            median,
            sorted.front(),
            sorted.back()
        };
    }


    double approximate95PercentHalfWidth(
        const Summary& summary,
        const std::size_t sampleCount
    )
    {
        if (sampleCount <= 1)
        {
            return 0.0;
        }

        const double standardError =
            summary.standardDeviation
            / std::sqrt(
                static_cast<double>(
                    sampleCount
                    )
            );

        // Normal approximation used here only as a descriptive
        // uncertainty summary.
        return
            1.96
            * standardError;
    }


    std::vector<double> pairedDifference(
        const std::vector<double>& a,
        const std::vector<double>& b
    )
    {
        if (
            a.size()
            != b.size()
            )
        {
            throw std::logic_error(
                "Cannot calculate paired difference from vectors of different size."
            );
        }

        std::vector<double> differences;

        differences.reserve(
            a.size()
        );

        for (
            std::size_t i = 0;
            i < a.size();
            ++i
            )
        {
            differences.push_back(
                a[i]
                - b[i]
            );
        }

        return differences;
    }


    void printSummary(
        const std::string& label,
        const std::vector<double>& values
    )
    {
        const Summary summary =
            summarize(
                values
            );

        const double halfWidth =
            approximate95PercentHalfWidth(
                summary,
                values.size()
            );

        std::cout
            << label
            << "\n"
            << "  mean:   "
            << summary.mean
            << "\n"
            << "  SD:     "
            << summary.standardDeviation
            << "\n"
            << "  median: "
            << summary.median
            << "\n"
            << "  range:  ["
            << summary.minimum
            << ", "
            << summary.maximum
            << "]\n"
            << "  approx 95% mean interval: ["
            << summary.mean - halfWidth
            << ", "
            << summary.mean + halfWidth
            << "]\n";
    }


    void printPairedComparison(
        const std::string& label,
        const std::vector<double>& a,
        const std::vector<double>& b
    )
    {
        const std::vector<double> differences =
            pairedDifference(
                a,
                b
            );

        const Summary summary =
            summarize(
                differences
            );

        const double halfWidth =
            approximate95PercentHalfWidth(
                summary,
                differences.size()
            );


        std::size_t positive =
            0;

        std::size_t negative =
            0;

        std::size_t tied =
            0;

        for (const double difference :
        differences)
        {
            if (difference > 0.0)
            {
                ++positive;
            }
            else if (difference < 0.0)
            {
                ++negative;
            }
            else
            {
                ++tied;
            }
        }


        std::cout
            << label
            << "\n"
            << "  paired mean difference: "
            << summary.mean
            << "\n"
            << "  SD:                     "
            << summary.standardDeviation
            << "\n"
            << "  median difference:      "
            << summary.median
            << "\n"
            << "  approx 95% interval:    ["
            << summary.mean - halfWidth
            << ", "
            << summary.mean + halfWidth
            << "]\n"
            << "  positive / negative / tie: "
            << positive
            << " / "
            << negative
            << " / "
            << tied
            << "\n";
    }


    // ============================================================
    // OUTPUT
    // ============================================================

    double viableMeanFitness(
        const ae::GenerationStatistics& statistics
    )
    {
        const double viableFraction =
            1.0
            - statistics
            .developmentFailureFraction;

        if (viableFraction <= 0.0)
        {
            return 0.0;
        }

        // Development-interface failures produce zero fitness in the
        // current simulation, so dividing total mean fitness by the
        // surviving fraction gives the mean among nonfailed organisms.
        return
            statistics.meanFitness
            / viableFraction;
    }


    void writeGenerationRow(
        std::ofstream& output,
        const std::size_t replicate,
        const std::uint64_t seed,
        const std::string& treatment,
        const ae::GenerationStatistics& statistics
    )
    {
        output
            << replicate
            << ','
            << seed
            << ','
            << treatment
            << ','
            << statistics.generation
            << ','
            << statistics.populationSize
            << ','
            << statistics.meanFitness
            << ','
            << viableMeanFitness(
                statistics
            )
            << ','
            << statistics.maximumFitness
            << ','
            << statistics.meanMaterial
            << ','
            << statistics.meanBoundary
            << ','
            << statistics.meanNetEnergy
            << ','
            << statistics.meanRegulatoryNodeCount
            << ','
            << statistics.meanRegulatoryInteractionCount
            << ','
            << statistics.meanRegulatoryNetworkDensity
            << ','
            << statistics.developmentFailureFraction
            << '\n';
    }


    void writeRunSummaryRow(
        std::ofstream& output,
        const std::size_t replicate,
        const std::uint64_t seed,
        const std::string& treatment,
        const RunOutcome& outcome
    )
    {
        output
            << replicate
            << ','
            << seed
            << ','
            << treatment
            << ','
            << outcome.initialMeanFitness
            << ','
            << outcome.finalMeanFitness
            << ','
            << outcome.finalViableMeanFitness
            << ','
            << outcome.finalMaximumFitness
            << ','
            << outcome.finalMeanMaterial
            << ','
            << outcome.finalMeanBoundary
            << ','
            << outcome.finalMeanNetEnergy
            << ','
            << outcome.finalMeanNodeCount
            << ','
            << outcome.finalMeanInteractionCount
            << ','
            << outcome.finalMeanNetworkDensity
            << ','
            << outcome.finalDevelopmentFailureFraction
            << ','
            << outcome.meanDevelopmentFailureFraction
            << ','
            << outcome.generationsObserved
            << ','
            << (
                outcome.extinct
                ? 1
                : 0
                )
            << '\n';
    }


    // ============================================================
    // RUN ONE TREATMENT
    // ============================================================

    RunOutcome runTreatment(
        const std::size_t replicate,
        const std::uint64_t seed,
        const std::string& treatmentName,
        ae::SimulationConfig config,
        std::ofstream& rawOutput
    )
    {
        ae::Environment environment{};

        environment.resourceAvailability =
            1.0;


        ae::Simulation simulation(
            environment,
            makeFounderProgram(),
            config,
            seed
        );


        RunOutcome outcome{};

        bool sawGeneration =
            false;

        ae::GenerationStatistics lastStatistics{};

        double failureFractionTotal =
            0.0;


        for (
            std::size_t generation = 0;
            generation < generationCount;
            ++generation
            )
        {
            if (simulation.extinct())
            {
                break;
            }

            const ae::GenerationStatistics statistics =
                simulation.step();


            if (!sawGeneration)
            {
                outcome.initialMeanFitness =
                    statistics.meanFitness;

                sawGeneration =
                    true;
            }


            writeGenerationRow(
                rawOutput,
                replicate,
                seed,
                treatmentName,
                statistics
            );


            failureFractionTotal +=
                statistics.developmentFailureFraction;

            lastStatistics =
                statistics;

            ++outcome.generationsObserved;
        }


        outcome.extinct =
            simulation.extinct();


        if (
            outcome.generationsObserved > 0
            )
        {
            outcome.meanDevelopmentFailureFraction =
                failureFractionTotal
                / static_cast<double>(
                    outcome.generationsObserved
                    );
        }


        if (!sawGeneration)
        {
            return outcome;
        }


        if (outcome.extinct)
        {
            // Extinction is assigned zero final population-level
            // performance at the requested experimental horizon.
            outcome.finalMeanFitness =
                0.0;

            outcome.finalViableMeanFitness =
                0.0;

            outcome.finalMaximumFitness =
                0.0;

            outcome.finalMeanMaterial =
                0.0;

            outcome.finalMeanBoundary =
                0.0;

            outcome.finalMeanNetEnergy =
                0.0;

            outcome.finalMeanNodeCount =
                0.0;

            outcome.finalMeanInteractionCount =
                0.0;

            outcome.finalMeanNetworkDensity =
                0.0;

            outcome.finalDevelopmentFailureFraction =
                1.0;

            return outcome;
        }


        outcome.finalMeanFitness =
            lastStatistics.meanFitness;

        outcome.finalViableMeanFitness =
            viableMeanFitness(
                lastStatistics
            );

        outcome.finalMaximumFitness =
            lastStatistics.maximumFitness;

        outcome.finalMeanMaterial =
            lastStatistics.meanMaterial;

        outcome.finalMeanBoundary =
            lastStatistics.meanBoundary;

        outcome.finalMeanNetEnergy =
            lastStatistics.meanNetEnergy;

        outcome.finalMeanNodeCount =
            lastStatistics.meanRegulatoryNodeCount;

        outcome.finalMeanInteractionCount =
            lastStatistics.meanRegulatoryInteractionCount;

        outcome.finalMeanNetworkDensity =
            lastStatistics.meanRegulatoryNetworkDensity;

        outcome.finalDevelopmentFailureFraction =
            lastStatistics.developmentFailureFraction;


        return outcome;
    }


    } // namespace

    // ============================================================
    // MAIN
    // ============================================================

    int main()
    {
        try
        {
            const auto startTime =
                std::chrono::steady_clock::now();


            std::filesystem::create_directories(
                "experiments/output"
            );


            std::ofstream rawOutput(
                "experiments/output/"
                "regulatory_selection_validation_raw.csv"
            );

            std::ofstream runOutput(
                "experiments/output/"
                "regulatory_selection_validation_runs.csv"
            );


            if (
                !rawOutput
                || !runOutput
                )
            {
                throw std::runtime_error(
                    "Could not open experiment output CSV."
                );
            }


            rawOutput
                << std::setprecision(17);

            runOutput
                << std::setprecision(17);


            rawOutput
                << "replicate,"
                << "seed,"
                << "treatment,"
                << "generation,"
                << "population_size,"
                << "mean_fitness,"
                << "viable_mean_fitness,"
                << "maximum_fitness,"
                << "mean_material,"
                << "mean_boundary,"
                << "mean_net_energy,"
                << "mean_node_count,"
                << "mean_interaction_count,"
                << "mean_network_density,"
                << "development_failure_fraction\n";


            runOutput
                << "replicate,"
                << "seed,"
                << "treatment,"
                << "initial_mean_fitness,"
                << "final_mean_fitness,"
                << "final_viable_mean_fitness,"
                << "final_maximum_fitness,"
                << "final_mean_material,"
                << "final_mean_boundary,"
                << "final_mean_net_energy,"
                << "final_mean_node_count,"
                << "final_mean_interaction_count,"
                << "final_mean_network_density,"
                << "final_development_failure_fraction,"
                << "mean_development_failure_fraction,"
                << "generations_observed,"
                << "extinct\n";


            // --------------------------------------------------------
            // Treatment result vectors.
            // --------------------------------------------------------

            std::vector<double> selectedFullFinal;
            std::vector<double> neutralFullFinal;
            std::vector<double> standingFinal;
            std::vector<double> selectedQuantFinal;
            std::vector<double> neutralQuantFinal;


            std::vector<double> selectedFullViableFinal;
            std::vector<double> neutralFullViableFinal;
            std::vector<double> selectedQuantViableFinal;
            std::vector<double> neutralQuantViableFinal;


            std::vector<double> selectedFullFailure;
            std::vector<double> neutralFullFailure;
            std::vector<double> selectedQuantFailure;
            std::vector<double> neutralQuantFailure;


            std::vector<double> selectedFullNodes;
            std::vector<double> neutralFullNodes;

            std::vector<double> selectedFullEdges;
            std::vector<double> neutralFullEdges;


            std::size_t selectedFullExtinctions =
                0;

            std::size_t neutralFullExtinctions =
                0;

            std::size_t standingExtinctions =
                0;

            std::size_t selectedQuantExtinctions =
                0;

            std::size_t neutralQuantExtinctions =
                0;


            std::cout
                << std::fixed
                << std::setprecision(6);


            std::cout
                << "AlienEvolution V0.2 full regulatory selection validation\n"
                << "Replicates: "
                << replicateCount
                << "\nGenerations: "
                << generationCount
                << "\nPopulation: "
                << populationSize
                << "\nTreatments: 5\n\n";


            for (
                std::size_t replicate = 0;
                replicate < replicateCount;
                ++replicate
                )
            {
                const std::uint64_t seed =
                    baseSeed
                    + static_cast<std::uint64_t>(
                        replicate
                        )
                    * 100003ULL;


                // ====================================================
                // A. SELECTION + FULL MUTATION
                // ====================================================

                ae::SimulationConfig selectedFullConfig =
                    makeBaseSimulationConfig();

                selectedFullConfig.selectionMode =
                    ae::SelectionMode::FitnessProportional;

                selectedFullConfig.offspringMutation =
                    makeFullMutationConfig();


                const RunOutcome selectedFull =
                    runTreatment(
                        replicate,
                        seed,
                        "selection_full_mutation",
                        selectedFullConfig,
                        rawOutput
                    );


                // ====================================================
                // B. NEUTRAL + FULL MUTATION
                // ====================================================

                ae::SimulationConfig neutralFullConfig =
                    makeBaseSimulationConfig();

                neutralFullConfig.selectionMode =
                    ae::SelectionMode::Uniform;

                neutralFullConfig.offspringMutation =
                    makeFullMutationConfig();


                const RunOutcome neutralFull =
                    runTreatment(
                        replicate,
                        seed,
                        "neutral_full_mutation",
                        neutralFullConfig,
                        rawOutput
                    );


                // ====================================================
                // C. SELECTION + STANDING VARIATION ONLY
                // ====================================================

                ae::SimulationConfig standingConfig =
                    makeBaseSimulationConfig();

                standingConfig.selectionMode =
                    ae::SelectionMode::FitnessProportional;

                standingConfig.offspringMutation =
                    ae::RegulatoryMutationGeneratorConfig{};


                const RunOutcome standing =
                    runTreatment(
                        replicate,
                        seed,
                        "selection_standing_only",
                        standingConfig,
                        rawOutput
                    );


                // ====================================================
                // D. SELECTION + QUANTITATIVE MUTATION ONLY
                // ====================================================

                ae::SimulationConfig selectedQuantConfig =
                    makeBaseSimulationConfig();

                selectedQuantConfig.selectionMode =
                    ae::SelectionMode::FitnessProportional;

                selectedQuantConfig.offspringMutation =
                    makeQuantitativeMutationConfig();


                const RunOutcome selectedQuant =
                    runTreatment(
                        replicate,
                        seed,
                        "selection_quantitative_only",
                        selectedQuantConfig,
                        rawOutput
                    );


                // ====================================================
                // E. NEUTRAL + QUANTITATIVE MUTATION ONLY
                // ====================================================

                ae::SimulationConfig neutralQuantConfig =
                    makeBaseSimulationConfig();

                neutralQuantConfig.selectionMode =
                    ae::SelectionMode::Uniform;

                neutralQuantConfig.offspringMutation =
                    makeQuantitativeMutationConfig();


                const RunOutcome neutralQuant =
                    runTreatment(
                        replicate,
                        seed,
                        "neutral_quantitative_only",
                        neutralQuantConfig,
                        rawOutput
                    );


                // ----------------------------------------------------
                // Verify exact matched initial conditions.
                // ----------------------------------------------------

                constexpr double initialTolerance =
                    1.0e-12;

                const double initial =
                    selectedFull.initialMeanFitness;

                if (
                    std::abs(
                        neutralFull.initialMeanFitness
                        - initial
                    )
                > initialTolerance
                    ||
                    std::abs(
                        standing.initialMeanFitness
                        - initial
                    )
            > initialTolerance
                    ||
                    std::abs(
                        selectedQuant.initialMeanFitness
                        - initial
                    )
                    > initialTolerance
                    ||
                    std::abs(
                        neutralQuant.initialMeanFitness
                        - initial
                    )
                    > initialTolerance
                    )
                {
                    throw std::runtime_error(
                        "Matched treatments did not begin from identical initial fitness."
                    );
                }


                // ----------------------------------------------------
                // Save run-level output.
                // ----------------------------------------------------

                writeRunSummaryRow(
                    runOutput,
                    replicate,
                    seed,
                    "selection_full_mutation",
                    selectedFull
                );

                writeRunSummaryRow(
                    runOutput,
                    replicate,
                    seed,
                    "neutral_full_mutation",
                    neutralFull
                );

                writeRunSummaryRow(
                    runOutput,
                    replicate,
                    seed,
                    "selection_standing_only",
                    standing
                );

                writeRunSummaryRow(
                    runOutput,
                    replicate,
                    seed,
                    "selection_quantitative_only",
                    selectedQuant
                );

                writeRunSummaryRow(
                    runOutput,
                    replicate,
                    seed,
                    "neutral_quantitative_only",
                    neutralQuant
                );


                // ----------------------------------------------------
                // Save vectors for aggregate analysis.
                // ----------------------------------------------------

                selectedFullFinal.push_back(
                    selectedFull.finalMeanFitness
                );

                neutralFullFinal.push_back(
                    neutralFull.finalMeanFitness
                );

                standingFinal.push_back(
                    standing.finalMeanFitness
                );

                selectedQuantFinal.push_back(
                    selectedQuant.finalMeanFitness
                );

                neutralQuantFinal.push_back(
                    neutralQuant.finalMeanFitness
                );


                selectedFullViableFinal.push_back(
                    selectedFull.finalViableMeanFitness
                );

                neutralFullViableFinal.push_back(
                    neutralFull.finalViableMeanFitness
                );

                selectedQuantViableFinal.push_back(
                    selectedQuant.finalViableMeanFitness
                );

                neutralQuantViableFinal.push_back(
                    neutralQuant.finalViableMeanFitness
                );


                selectedFullFailure.push_back(
                    selectedFull.meanDevelopmentFailureFraction
                );

                neutralFullFailure.push_back(
                    neutralFull.meanDevelopmentFailureFraction
                );

                selectedQuantFailure.push_back(
                    selectedQuant.meanDevelopmentFailureFraction
                );

                neutralQuantFailure.push_back(
                    neutralQuant.meanDevelopmentFailureFraction
                );


                selectedFullNodes.push_back(
                    selectedFull.finalMeanNodeCount
                );

                neutralFullNodes.push_back(
                    neutralFull.finalMeanNodeCount
                );

                selectedFullEdges.push_back(
                    selectedFull.finalMeanInteractionCount
                );

                neutralFullEdges.push_back(
                    neutralFull.finalMeanInteractionCount
                );


                if (selectedFull.extinct)
                {
                    ++selectedFullExtinctions;
                }

                if (neutralFull.extinct)
                {
                    ++neutralFullExtinctions;
                }

                if (standing.extinct)
                {
                    ++standingExtinctions;
                }

                if (selectedQuant.extinct)
                {
                    ++selectedQuantExtinctions;
                }

                if (neutralQuant.extinct)
                {
                    ++neutralQuantExtinctions;
                }


                // Progress only, to keep terminal output manageable.
                std::cout
                    << "Completed replicate "
                    << replicate + 1
                    << "/"
                    << replicateCount
                    << '\n';
            }


            rawOutput.close();
            runOutput.close();


            // ========================================================
            // AGGREGATE RESULTS
            // ========================================================

            std::cout
                << "\n\n"
                << "=============================================\n"
                << "FINAL FITNESS BY TREATMENT\n"
                << "=============================================\n\n";


            printSummary(
                "Selection + full mutation",
                selectedFullFinal
            );

            std::cout << '\n';

            printSummary(
                "Neutral + full mutation",
                neutralFullFinal
            );

            std::cout << '\n';

            printSummary(
                "Selection + standing variation only",
                standingFinal
            );

            std::cout << '\n';

            printSummary(
                "Selection + quantitative mutation only",
                selectedQuantFinal
            );

            std::cout << '\n';

            printSummary(
                "Neutral + quantitative mutation only",
                neutralQuantFinal
            );


            // ========================================================
            // PAIRED SELECTION TESTS
            // ========================================================

            std::cout
                << "\n\n"
                << "=============================================\n"
                << "PAIRED COMPARISONS\n"
                << "=============================================\n\n";


            printPairedComparison(
                "Selection effect under FULL mutation"
                "  [selected full - neutral full]",
                selectedFullFinal,
                neutralFullFinal
            );

            std::cout << '\n';


            printPairedComparison(
                "Selection effect under QUANTITATIVE-only mutation"
                "  [selected quant - neutral quant]",
                selectedQuantFinal,
                neutralQuantFinal
            );

            std::cout << '\n';


            printPairedComparison(
                "Ongoing mutation contribution under selection"
                "  [selected full - standing only]",
                selectedFullFinal,
                standingFinal
            );

            std::cout << '\n';


            printPairedComparison(
                "Full versus quantitative-only mutation under selection"
                "  [selected full - selected quant]",
                selectedFullFinal,
                selectedQuantFinal
            );


            // ========================================================
            // FITNESS AMONG NONFAILED ORGANISMS
            // ========================================================

            std::cout
                << "\n\n"
                << "=============================================\n"
                << "FINAL FITNESS EXCLUDING DEVELOPMENT FAILURES\n"
                << "=============================================\n\n";


            printPairedComparison(
                "Selection effect among developmentally viable organisms"
                " under FULL mutation",
                selectedFullViableFinal,
                neutralFullViableFinal
            );

            std::cout << '\n';


            printPairedComparison(
                "Selection effect among developmentally viable organisms"
                " under QUANTITATIVE-only mutation",
                selectedQuantViableFinal,
                neutralQuantViableFinal
            );


            // ========================================================
            // DEVELOPMENTAL FAILURE
            // ========================================================

            std::cout
                << "\n\n"
                << "=============================================\n"
                << "MEAN DEVELOPMENT-FAILURE FRACTION\n"
                << "=============================================\n\n";


            printSummary(
                "Selection + full mutation",
                selectedFullFailure
            );

            std::cout << '\n';

            printSummary(
                "Neutral + full mutation",
                neutralFullFailure
            );

            std::cout << '\n';

            printSummary(
                "Selection + quantitative-only mutation",
                selectedQuantFailure
            );

            std::cout << '\n';

            printSummary(
                "Neutral + quantitative-only mutation",
                neutralQuantFailure
            );


            // ========================================================
            // TOPOLOGY
            // ========================================================

            std::cout
                << "\n\n"
                << "=============================================\n"
                << "FINAL NETWORK TOPOLOGY\n"
                << "=============================================\n\n";


            printSummary(
                "Selection + full mutation: mean node count",
                selectedFullNodes
            );

            std::cout << '\n';

            printSummary(
                "Neutral + full mutation: mean node count",
                neutralFullNodes
            );

            std::cout << '\n';

            printSummary(
                "Selection + full mutation: mean edge count",
                selectedFullEdges
            );

            std::cout << '\n';

            printSummary(
                "Neutral + full mutation: mean edge count",
                neutralFullEdges
            );


            // ========================================================
            // EXTINCTIONS
            // ========================================================

            std::cout
                << "\n\n"
                << "=============================================\n"
                << "EXTINCTIONS\n"
                << "=============================================\n\n"
                << "Selection + full mutation:       "
                << selectedFullExtinctions
                << "/"
                << replicateCount
                << '\n'
                << "Neutral + full mutation:         "
                << neutralFullExtinctions
                << "/"
                << replicateCount
                << '\n'
                << "Selection + standing only:       "
                << standingExtinctions
                << "/"
                << replicateCount
                << '\n'
                << "Selection + quantitative only:   "
                << selectedQuantExtinctions
                << "/"
                << replicateCount
                << '\n'
                << "Neutral + quantitative only:     "
                << neutralQuantExtinctions
                << "/"
                << replicateCount
                << '\n';


            // ========================================================
            // RUNTIME
            // ========================================================

            const auto endTime =
                std::chrono::steady_clock::now();

            const double elapsedSeconds =
                std::chrono::duration<double>(
                    endTime - startTime
                ).count();


            std::cout
                << "\nRuntime: "
                << elapsedSeconds
                << " seconds\n\n"
                << "Generation-level data:\n"
                << "  experiments/output/"
                << "regulatory_selection_validation_raw.csv\n\n"
                << "Run-level data:\n"
                << "  experiments/output/"
                << "regulatory_selection_validation_runs.csv\n";


            return 0;
        }
        catch (const std::exception& error)
        {
            std::cerr
                << "EXPERIMENT FAILURE: "
                << error.what()
                << '\n';

            return 1;
        }
    }