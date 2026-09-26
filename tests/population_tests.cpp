#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/evolution/Population.hpp"
#include "alien_evolution/genetics/RegulatoryMutationGenerator.hpp"
#include "alien_evolution/genetics/RegulatoryProgram.hpp"

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
                    1.0,
                    1.0
                },
                {
                    2,
                    0.0,
                    1.0,
                    1.0
                }
            },
        {
            {
                1,
                2,
                2.0,
                0.5,
                2.0
            }
        }
        );
    }

} // namespace

int main()
{
    try
    {
        const ae::RegulatoryProgram founder =
            makeFounder();

        // --------------------------------------------------------
        // Test 1:
        // Zero initial mutation produces exact founder copies.
        // --------------------------------------------------------

        ae::Random random(
            12345
        );

        const ae::RegulatoryMutationGeneratorConfig noVariation{};

        const ae::Population population(
            10,
            founder,
            random,
            noVariation
        );

        require(
            population.size() == 10,
            "Population has incorrect size."
        );

        require(
            !population.empty(),
            "Initialized population is unexpectedly empty."
        );

        for (
            const ae::Organism& organism :
            population.organisms()
            )
        {
            require(
                organism.regulatoryProgram().nodeCount()
                == founder.nodeCount(),
                "Founder node count was not inherited."
            );

            require(
                organism.regulatoryProgram().interactionCount()
                == founder.interactionCount(),
                "Founder interaction count was not inherited."
            );

            require(
                !organism.hasPhenotype(),
                "New organism unexpectedly has phenotype."
            );

            require(
                !organism.hasFitness(),
                "New organism unexpectedly has fitness."
            );
        }


        // --------------------------------------------------------
        // Test 2:
        // Explicit organism-vector constructor may represent
        // extinction with an empty population.
        // --------------------------------------------------------

        const ae::Population emptyPopulation(
            std::vector<ae::Organism>{}
        );

        require(
            emptyPopulation.empty(),
            "Explicit empty population is not empty."
        );

        require(
            emptyPopulation.size() == 0,
            "Explicit empty population has nonzero size."
        );


        // --------------------------------------------------------
        // Test 3:
        // Identical seeds produce identical initialized
        // populations when mutation is enabled.
        // --------------------------------------------------------

        ae::RegulatoryMutationGeneratorConfig mutationConfig{};

        mutationConfig.rates
            .nodeKineticPerNode =
            0.5;

        mutationConfig.quantitativeEffects
            .nodeBasalProductionLogStdDev =
            0.2;

        mutationConfig.quantitativeEffects
            .nodeDegradationLogStdDev =
            0.2;

        ae::Random randomA(
            777
        );

        ae::Random randomB(
            777
        );

        const ae::Population populationA(
            25,
            founder,
            randomA,
            mutationConfig
        );

        const ae::Population populationB(
            25,
            founder,
            randomB,
            mutationConfig
        );

        require(
            populationA.size()
            == populationB.size(),
            "Identical seeds produced populations of different size."
        );

        for (
            std::size_t i = 0;
            i < populationA.size();
            ++i
            )
        {
            const auto& programA =
                populationA.at(i)
                .regulatoryProgram();

            const auto& programB =
                populationB.at(i)
                .regulatoryProgram();

            require(
                programA.nodeCount()
                == programB.nodeCount(),
                "Identical seeds produced different node counts."
            );

            require(
                programA.interactionCount()
                == programB.interactionCount(),
                "Identical seeds produced different interaction counts."
            );

            for (
                std::size_t node = 0;
                node < programA.nodeCount();
                ++node
                )
            {
                require(
                    programA.nodes()[node].id
                    == programB.nodes()[node].id
                    && programA.nodes()[node].basalProductionRate
                    == programB.nodes()[node].basalProductionRate
                    && programA.nodes()[node].degradationRate
                    == programB.nodes()[node].degradationRate,
                    "Identical seeds produced different regulatory nodes."
                );
            }
        }


        // --------------------------------------------------------
        // Test 4:
        // Zero initial population is rejected.
        // --------------------------------------------------------

        bool threw =
            false;

        try
        {
            ae::Random invalidRandom(
                1
            );

            const ae::Population invalid(
                0,
                founder,
                invalidRandom,
                noVariation
            );
        }
        catch (const std::invalid_argument&)
        {
            threw =
                true;
        }

        require(
            threw,
            "Zero-sized initialized population was accepted."
        );


        std::cout
            << "All population tests passed.\n";

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