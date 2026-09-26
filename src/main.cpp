#include <iomanip>
#include <iostream>

#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/genetics/RegulatoryProgram.hpp"
#include "alien_evolution/simulation/Simulation.hpp"

int main()
{
    try
    {
        // --------------------------------------------------------
        // Minimal V0.2 founder regulatory program.
        //
        // Node 1: local-material input
        // Node 2: resource input
        // Node 3: deposition effector
        // --------------------------------------------------------

        const ae::RegulatoryProgram founderProgram(
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

        ae::Environment environment{};

        environment.resourceAvailability =
            1.0;

        ae::SimulationConfig config{};

        config.populationSize =
            50;

        config.developmentWidth =
            25;

        config.developmentHeight =
            25;

        config.developmentSteps =
            8;

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
            10;

        config.development.outputHalfSaturation =
            0.2;

        config.development.outputCooperativity =
            2.0;

        config.development.depositionRateScale =
            0.25;

        // V0.2 deliberately does not install arbitrary
        // biological mutation defaults.
        //
        // These values are explicit demonstration settings,
        // not biological constants.
        config.offspringMutation
            .rates
            .nodeKineticPerNode =
            0.02;

        config.offspringMutation
            .rates
            .interactionParameterPerInteraction =
            0.02;

        config.offspringMutation
            .rates
            .interactionGainPerAbsentPair =
            0.002;

        config.offspringMutation
            .rates
            .interactionLossPerInteraction =
            0.002;

        config.offspringMutation
            .rates
            .regulatoryUnitDuplicationPerNode =
            0.001;

        config.offspringMutation
            .rates
            .regulatoryNodeLossPerDeletableNode =
            0.001;

        config.offspringMutation
            .quantitativeEffects
            .nodeBasalProductionLogStdDev =
            0.05;

        config.offspringMutation
            .quantitativeEffects
            .nodeDegradationLogStdDev =
            0.05;

        config.offspringMutation
            .quantitativeEffects
            .interactionFoldChangeLogStdDev =
            0.05;

        config.offspringMutation
            .quantitativeEffects
            .interactionHalfSaturationLogStdDev =
            0.05;

        config.offspringMutation
            .quantitativeEffects
            .interactionCooperativityLogStdDev =
            0.05;

        config.offspringMutation
            .interactionGain
            .foldChangeLogStdDev =
            0.25;

        config.offspringMutation
            .interactionGain
            .referenceHalfSaturation =
            0.5;

        config.offspringMutation
            .interactionGain
            .halfSaturationLogStdDev =
            0.10;

        config.offspringMutation
            .interactionGain
            .referenceCooperativity =
            2.0;

        config.offspringMutation
            .interactionGain
            .cooperativityLogStdDev =
            0.10;

        constexpr std::uint64_t seed =
            12345;

        ae::Simulation simulation(
            environment,
            founderProgram,
            config,
            seed
        );

        const auto history =
            simulation.run(
                20
            );

        std::cout
            << std::fixed
            << std::setprecision(4);

        std::cout
            << "AlienEvolution V0.2 regulatory simulation\n"
            << "Seed: "
            << seed
            << "\n\n";

        std::cout
            << "Gen"
            << "\tPop"
            << "\tMeanFit"
            << "\tMaxFit"
            << "\tMaterial"
            << "\tBoundary"
            << "\tNodes"
            << "\tEdges"
            << "\tDensity"
            << "\tDevFail\n";

        for (const auto& statistics :
            history)
        {
            std::cout
                << statistics.generation
                << '\t'
                << statistics.populationSize
                << '\t'
                << statistics.meanFitness
                << '\t'
                << statistics.maximumFitness
                << '\t'
                << statistics.meanMaterial
                << '\t'
                << statistics.meanBoundary
                << '\t'
                << statistics.meanRegulatoryNodeCount
                << '\t'
                << statistics.meanRegulatoryInteractionCount
                << '\t'
                << statistics.meanRegulatoryNetworkDensity
                << '\t'
                << statistics.developmentFailureFraction
                << '\n';
        }

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr
            << "ERROR: "
            << error.what()
            << '\n';

        return 1;
    }
}