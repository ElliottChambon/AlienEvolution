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
        5;

    constexpr std::size_t generationCount =
        30;

    constexpr std::size_t populationSize =
        20;

    constexpr std::uint64_t baseSeed =
        20260926ULL;

    struct RunOutcome
    {
        double initialMeanFitness = 0.0;
        double finalMeanFitness = 0.0;
        double finalMaximumFitness = 0.0;

        double finalMeanMaterial = 0.0;
        double finalMeanBoundary = 0.0;
        double finalMeanNetEnergy = 0.0;

        double finalMeanNodeCount = 0.0;
        double finalMeanInteractionCount = 0.0;
        double finalMeanNetworkDensity = 0.0;

        double finalDevelopmentFailureFraction = 0.0;

        std::size_t generationsObserved = 0;

        bool extinct = false;
    };

    struct Summary
    {
        double mean = 0.0;
        double standardDeviation = 0.0;
    };

    ae::RegulatoryProgram makeFounderProgram()
    {
        return ae::RegulatoryProgram(
            {
                {
                    1,
                    0.0,
                    0.0,
                    1.0
                },
                {
                    2,
                    0.0,
                    0.0,
                    1.0
                },
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

    ae::RegulatoryMutationGeneratorConfig
        makeMutationConfig()
    {
        ae::RegulatoryMutationGeneratorConfig config{};

        // --------------------------------------------------------
        // Explicit V0.2 experimental null-model settings.
        //
        // These are NOT biological constants or calibrated
        // mutation rates.
        // --------------------------------------------------------

        config.rates.nodeKineticPerNode =
            0.02;

        config.rates.interactionParameterPerInteraction =
            0.02;

        config.rates.interactionGainPerAbsentPair =
            0.002;

        config.rates.interactionLossPerInteraction =
            0.002;

        config.rates.regulatoryUnitDuplicationPerNode =
            0.001;

        config.rates.regulatoryNodeLossPerDeletableNode =
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


        config.development.localMaterialInputNodeId =
            1;

        config.development.resourceInputNodeId =
            2;

        config.development.depositionOutputNodeId =
            3;

        config.development.neighborhoodLengthScale =
            1.0;

        config.development.regulatoryTimeStep =
            0.05;

        config.development.regulatoryStepsPerDevelopmentStep =
            6;

        config.development.outputHalfSaturation =
            0.2;

        config.development.outputCooperativity =
            2.0;

        config.development.depositionRateScale =
            0.25;


        // All treatments receive identical initial standing
        // variation.
        config.initialVariation =
            makeMutationConfig();

        return config;
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

            lastStatistics =
                statistics;

            ++outcome.generationsObserved;
        }

        outcome.extinct =
            simulation.extinct();

        if (!sawGeneration)
        {
            outcome.finalMeanFitness =
                0.0;

            return outcome;
        }

        if (outcome.extinct)
        {
            // At the requested experimental horizon, an extinct
            // lineage has zero population-level reproductive output.
            outcome.finalMeanFitness =
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

        return {
            mean,
            standardDeviation
        };
    }

    void printSummaryLine(
        const std::string& name,
        const std::vector<double>& initialFitness,
        const std::vector<double>& finalFitness
    )
    {
        const Summary initial =
            summarize(
                initialFitness
            );

        const Summary final =
            summarize(
                finalFitness
            );

        std::vector<double> changes;

        changes.reserve(
            finalFitness.size()
        );

        for (
            std::size_t i = 0;
            i < finalFitness.size();
            ++i
            )
        {
            changes.push_back(
                finalFitness[i]
                - initialFitness[i]
            );
        }

        const Summary change =
            summarize(
                changes
            );

        std::cout
            << name
            << "\n"
            << "  initial mean fitness: "
            << initial.mean
            << "\n"
            << "  final mean fitness:   "
            << final.mean
            << "  SD "
            << final.standardDeviation
            << "\n"
            << "  mean change:          "
            << change.mean
            << "  SD "
            << change.standardDeviation
            << "\n";
    }

} // namespace

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

        if (!rawOutput)
        {
            throw std::runtime_error(
                "Could not open raw experiment CSV."
            );
        }

        rawOutput
            << std::setprecision(17);

        rawOutput
            << "replicate,"
            << "seed,"
            << "treatment,"
            << "generation,"
            << "population_size,"
            << "mean_fitness,"
            << "maximum_fitness,"
            << "mean_material,"
            << "mean_boundary,"
            << "mean_net_energy,"
            << "mean_node_count,"
            << "mean_interaction_count,"
            << "mean_network_density,"
            << "development_failure_fraction\n";


        std::vector<double> selectedInitial;
        std::vector<double> selectedFinal;

        std::vector<double> neutralInitial;
        std::vector<double> neutralFinal;

        std::vector<double> standingInitial;
        std::vector<double> standingFinal;

        std::vector<double> selectedMinusNeutral;

        std::vector<double> selectedMinusStanding;

        std::size_t selectedHigherThanNeutral =
            0;

        std::size_t selectedHigherThanStanding =
            0;

        std::size_t neutralHigherThanSelected =
            0;

        std::size_t standingHigherThanSelected =
            0;


        std::cout
            << std::fixed
            << std::setprecision(6);

        std::cout
            << "AlienEvolution V0.2 regulatory selection validation\n"
            << "Replicates: "
            << replicateCount
            << "\nGenerations: "
            << generationCount
            << "\nPopulation: "
            << populationSize
            << "\n\n";


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


            // ----------------------------------------------------
            // Treatment A:
            // fitness-proportional selection + ongoing mutation
            // ----------------------------------------------------

            ae::SimulationConfig selectedConfig =
                makeBaseSimulationConfig();

            selectedConfig.selectionMode =
                ae::SelectionMode::FitnessProportional;

            selectedConfig.offspringMutation =
                makeMutationConfig();

            const RunOutcome selected =
                runTreatment(
                    replicate,
                    seed,
                    "selection_mutation",
                    selectedConfig,
                    rawOutput
                );


            // ----------------------------------------------------
            // Treatment B:
            // neutral reproduction + identical ongoing mutation
            // ----------------------------------------------------

            ae::SimulationConfig neutralConfig =
                makeBaseSimulationConfig();

            neutralConfig.selectionMode =
                ae::SelectionMode::Uniform;

            neutralConfig.offspringMutation =
                makeMutationConfig();

            const RunOutcome neutral =
                runTreatment(
                    replicate,
                    seed,
                    "neutral_mutation",
                    neutralConfig,
                    rawOutput
                );


            // ----------------------------------------------------
            // Treatment C:
            // selection acts on identical initial standing
            // variation but no new mutation occurs afterward.
            // ----------------------------------------------------

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
                    "selection_standing_variation",
                    standingConfig,
                    rawOutput
                );


            // Because the three simulations use the same founder,
            // initial-variation model, and seed, generation zero
            // should be paired exactly.
            constexpr double initialTolerance =
                1.0e-12;

            if (
                std::abs(
                    selected.initialMeanFitness
                    - neutral.initialMeanFitness
                )
            > initialTolerance
                ||
                std::abs(
                    selected.initialMeanFitness
                    - standing.initialMeanFitness
                )
            > initialTolerance
                )
            {
                throw std::runtime_error(
                    "Matched treatments did not begin from the same initial fitness."
                );
            }


            selectedInitial.push_back(
                selected.initialMeanFitness
            );

            selectedFinal.push_back(
                selected.finalMeanFitness
            );

            neutralInitial.push_back(
                neutral.initialMeanFitness
            );

            neutralFinal.push_back(
                neutral.finalMeanFitness
            );

            standingInitial.push_back(
                standing.initialMeanFitness
            );

            standingFinal.push_back(
                standing.finalMeanFitness
            );


            const double selectionDifference =
                selected.finalMeanFitness
                - neutral.finalMeanFitness;

            const double mutationDifference =
                selected.finalMeanFitness
                - standing.finalMeanFitness;

            selectedMinusNeutral.push_back(
                selectionDifference
            );

            selectedMinusStanding.push_back(
                mutationDifference
            );


            if (selectionDifference > 0.0)
            {
                ++selectedHigherThanNeutral;
            }
            else if (selectionDifference < 0.0)
            {
                ++neutralHigherThanSelected;
            }

            if (mutationDifference > 0.0)
            {
                ++selectedHigherThanStanding;
            }
            else if (mutationDifference < 0.0)
            {
                ++standingHigherThanSelected;
            }


            std::cout
                << "Replicate "
                << replicate
                << " seed "
                << seed
                << "\n"
                << "  initial:              "
                << selected.initialMeanFitness
                << "\n"
                << "  selection+mutation:   "
                << selected.finalMeanFitness
                << "\n"
                << "  neutral+mutation:     "
                << neutral.finalMeanFitness
                << "\n"
                << "  selection+standing:   "
                << standing.finalMeanFitness
                << "\n"
                << "  selected - neutral:   "
                << selectionDifference
                << "\n"
                << "  selected - standing:  "
                << mutationDifference
                << "\n\n";
        }


        rawOutput.close();


        std::cout
            << "\n===== PILOT SUMMARY =====\n\n";

        printSummaryLine(
            "Selection + mutation",
            selectedInitial,
            selectedFinal
        );

        printSummaryLine(
            "Neutral + mutation",
            neutralInitial,
            neutralFinal
        );

        printSummaryLine(
            "Selection + standing variation",
            standingInitial,
            standingFinal
        );


        const Summary selectedNeutralSummary =
            summarize(
                selectedMinusNeutral
            );

        const Summary selectedStandingSummary =
            summarize(
                selectedMinusStanding
            );

        std::cout
            << "\nPaired final-fitness differences\n"
            << "  selected - neutral mean:  "
            << selectedNeutralSummary.mean
            << "  SD "
            << selectedNeutralSummary.standardDeviation
            << "\n"
            << "  selected higher: "
            << selectedHigherThanNeutral
            << "/"
            << replicateCount
            << "\n"
            << "  neutral higher:  "
            << neutralHigherThanSelected
            << "/"
            << replicateCount
            << "\n\n"
            << "  selected - standing mean: "
            << selectedStandingSummary.mean
            << "  SD "
            << selectedStandingSummary.standardDeviation
            << "\n"
            << "  selected higher: "
            << selectedHigherThanStanding
            << "/"
            << replicateCount
            << "\n"
            << "  standing higher: "
            << standingHigherThanSelected
            << "/"
            << replicateCount
            << "\n";


        const auto endTime =
            std::chrono::steady_clock::now();

        const double elapsedSeconds =
            std::chrono::duration<double>(
                endTime - startTime
            ).count();

        std::cout
            << "\nRuntime: "
            << elapsedSeconds
            << " seconds\n"
            << "\nRaw generation data written to:\n"
            << "experiments/output/"
            << "regulatory_selection_validation_raw.csv\n";


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