#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>

#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/genetics/RegulatoryProgram.hpp"
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
            throw std::runtime_error(
                message
            );
        }
    }

    ae::RegulatoryProgram makeFounder()
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

    ae::SimulationConfig makeConfig()
    {
        ae::SimulationConfig config{};

        config.populationSize =
            20;

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
            10;

        config.development.outputHalfSaturation =
            0.2;

        config.development.outputCooperativity =
            2.0;

        config.development.depositionRateScale =
            0.25;

        return config;
    }

} // namespace

int main()
{
    try
    {
        ae::Environment environment{};

        environment.resourceAvailability =
            1.0;

        const ae::RegulatoryProgram founder =
            makeFounder();

        const ae::SimulationConfig config =
            makeConfig();


        // --------------------------------------------------------
        // Test 1:
        // Simulation initializes regulatory organisms.
        // --------------------------------------------------------

        ae::Simulation simulation(
            environment,
            founder,
            config,
            12345
        );

        require(
            simulation.population().size()
            == config.populationSize,
            "Simulation initialized wrong population size."
        );

        require(
            simulation.generation() == 0,
            "Simulation began at incorrect generation."
        );

        require(
            !simulation.extinct(),
            "Fresh simulation is unexpectedly extinct."
        );


        // --------------------------------------------------------
        // Test 2:
        // One simulation step develops and evaluates the current
        // population.
        // --------------------------------------------------------

        const ae::GenerationStatistics generationZero =
            simulation.step();

        require(
            generationZero.generation == 0,
            "Generation statistics have incorrect generation number."
        );

        require(
            generationZero.populationSize
            == config.populationSize,
            "Generation statistics have incorrect population size."
        );

        require(
            generationZero.meanMaterial > 1.0,
            "Regulatory simulation failed to develop material."
        );

        require(
            generationZero.meanRegulatoryNodeCount == 3.0,
            "Incorrect mean regulatory node count."
        );

        require(
            generationZero.meanRegulatoryInteractionCount == 2.0,
            "Incorrect mean regulatory interaction count."
        );

        require(
            std::abs(
                generationZero.meanRegulatoryNetworkDensity
                - (2.0 / 9.0)
            )
            < 1.0e-12,
            "Incorrect regulatory network density."
        );

        require(
            generationZero.developmentFailureCount == 0,
            "Founder population unexpectedly failed development."
        );

        require(
            simulation.generation() == 1,
            "Simulation generation did not advance."
        );


        // --------------------------------------------------------
        // Test 3:
        // Zero-mutation simulation preserves network topology.
        // --------------------------------------------------------

        const ae::GenerationStatistics generationOne =
            simulation.step();

        require(
            generationOne.meanRegulatoryNodeCount == 3.0,
            "Network node count changed despite zero mutation rates."
        );

        require(
            generationOne.meanRegulatoryInteractionCount == 2.0,
            "Network edge count changed despite zero mutation rates."
        );


        // --------------------------------------------------------
        // Test 4:
        // Identical seeds produce identical histories.
        // --------------------------------------------------------

        ae::Simulation simulationA(
            environment,
            founder,
            config,
            98765
        );

        ae::Simulation simulationB(
            environment,
            founder,
            config,
            98765
        );

        const auto historyA =
            simulationA.run(
                4
            );

        const auto historyB =
            simulationB.run(
                4
            );

        require(
            historyA.size()
            == historyB.size(),
            "Identical simulations produced histories of different length."
        );

        for (
            std::size_t i = 0;
            i < historyA.size();
            ++i
            )
        {
            require(
                historyA[i].meanFitness
                == historyB[i].meanFitness
                && historyA[i].maximumFitness
                == historyB[i].maximumFitness
                && historyA[i].meanMaterial
                == historyB[i].meanMaterial
                && historyA[i].meanBoundary
                == historyB[i].meanBoundary
                && historyA[i].meanRegulatoryNodeCount
                == historyB[i].meanRegulatoryNodeCount
                && historyA[i].meanRegulatoryInteractionCount
                == historyB[i].meanRegulatoryInteractionCount,
                "Identical seeds produced different simulation histories."
            );
        }


        // --------------------------------------------------------
        // Test 5:
        // Missing required developmental interface is rejected for
        // the founder.
        // --------------------------------------------------------

        bool threw =
            false;

        try
        {
            const ae::RegulatoryProgram invalidFounder(
                {
                    {
                        1,
                        0.0,
                        1.0,
                        1.0
                    }
                },
                {}
            );

            ae::Simulation invalidSimulation(
                environment,
                invalidFounder,
                config,
                1
            );
        }
        catch (const std::invalid_argument&)
        {
            threw =
                true;
        }

        require(
            threw,
            "Founder missing developmental interface was accepted."
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