#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/evolution/Population.hpp"
#include "alien_evolution/evolution/Reproduction.hpp"
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

    ae::RegulatoryProgram makeProgram(
        const double basalProductionRate
    )
    {
        return ae::RegulatoryProgram(
            {
                {
                    1,
                    0.0,
                    basalProductionRate,
                    1.0
                }
            },
            {}
        );
    }

} // namespace

int main()
{
    try
    {
        const ae::RegulatoryMutationGeneratorConfig noMutation{};


        // --------------------------------------------------------
        // Test 1:
        // Fitness-proportional reproduction copies the only
        // reproductively viable lineage.
        // --------------------------------------------------------

        ae::Organism unfit(
            makeProgram(
                1.0
            )
        );

        ae::Organism fit(
            makeProgram(
                5.0
            )
        );

        unfit.setFitness(
            0.0
        );

        fit.setFitness(
            10.0
        );

        ae::Population parents(
            std::vector<ae::Organism>{
            unfit,
                fit
        }
        );

        ae::Random selectedRandom(
            1234
        );

        const ae::Population selectedOffspring =
            ae::reproducePopulation(
                parents,
                50,
                selectedRandom,
                noMutation,
                ae::SelectionMode::FitnessProportional
            );

        require(
            selectedOffspring.size() == 50,
            "Incorrect offspring population size."
        );

        for (
            const ae::Organism& child :
            selectedOffspring.organisms()
            )
        {
            require(
                child.regulatoryProgram()
                .node(1)
                .basalProductionRate
                == 5.0,
                "Fitness-proportional selection reproduced zero-fitness parent."
            );

            require(
                !child.hasFitness(),
                "New offspring unexpectedly inherited evaluated fitness."
            );

            require(
                !child.hasPhenotype(),
                "New offspring unexpectedly inherited phenotype."
            );
        }


        // --------------------------------------------------------
        // Test 2:
        // Uniform reproduction permits zero-fitness parents.
        // --------------------------------------------------------

        ae::Random uniformRandom(
            9999
        );

        const ae::Population uniformOffspring =
            ae::reproducePopulation(
                parents,
                200,
                uniformRandom,
                noMutation,
                ae::SelectionMode::Uniform
            );

        bool sawUnfitLineage =
            false;

        bool sawFitLineage =
            false;

        for (
            const ae::Organism& child :
            uniformOffspring.organisms()
            )
        {
            const double value =
                child.regulatoryProgram()
                .node(1)
                .basalProductionRate;

            if (value == 1.0)
            {
                sawUnfitLineage =
                    true;
            }

            if (value == 5.0)
            {
                sawFitLineage =
                    true;
            }
        }

        require(
            sawUnfitLineage
            && sawFitLineage,
            "Uniform selection failed to sample both parents."
        );


        // --------------------------------------------------------
        // Test 3:
        // Fitness-proportional reproduction with total fitness
        // equal to zero produces extinction.
        // --------------------------------------------------------

        ae::Organism zeroA(
            makeProgram(
                1.0
            )
        );

        ae::Organism zeroB(
            makeProgram(
                2.0
            )
        );

        zeroA.setFitness(
            0.0
        );

        zeroB.setFitness(
            0.0
        );

        const ae::Population zeroParents(
            std::vector<ae::Organism>{
            zeroA,
                zeroB
        }
        );

        ae::Random extinctionRandom(
            2222
        );

        const ae::Population extinct =
            ae::reproducePopulation(
                zeroParents,
                20,
                extinctionRandom,
                noMutation,
                ae::SelectionMode::FitnessProportional
            );

        require(
            extinct.empty(),
            "Zero-fitness population failed to go extinct."
        );


        // --------------------------------------------------------
        // Test 4:
        // Regulatory mutation generator is actually used during
        // reproduction.
        // --------------------------------------------------------

        ae::Organism mutationParent(
            makeProgram(
                1.0
            )
        );

        mutationParent.setFitness(
            1.0
        );

        const ae::Population mutationParents(
            std::vector<ae::Organism>{
            mutationParent
        }
        );

        ae::RegulatoryMutationGeneratorConfig mutationConfig{};

        mutationConfig.rates
            .nodeKineticPerNode =
            100.0;

        mutationConfig.quantitativeEffects
            .nodeBasalProductionLogStdDev =
            0.2;

        mutationConfig.quantitativeEffects
            .nodeDegradationLogStdDev =
            0.2;

        ae::Random mutationRandom(
            3333
        );

        const ae::Population mutated =
            ae::reproducePopulation(
                mutationParents,
                1,
                mutationRandom,
                mutationConfig,
                ae::SelectionMode::FitnessProportional
            );

        require(
            mutated.size() == 1,
            "Mutation reproduction produced wrong offspring count."
        );

        const auto& childNode =
            mutated.at(0)
            .regulatoryProgram()
            .node(1);

        require(
            childNode.basalProductionRate
            != 1.0
            || childNode.degradationRate
            != 1.0,
            "Regulatory mutation was not applied during reproduction."
        );


        // --------------------------------------------------------
        // Test 5:
        // Unevaluated parents cannot reproduce.
        // --------------------------------------------------------

        bool threw =
            false;

        try
        {
            const ae::Population unevaluated(
                std::vector<ae::Organism>{
                ae::Organism(
                    makeProgram(
                        1.0
                    )
                )
            }
            );

            ae::Random random(
                1
            );

            const auto result =
                ae::reproducePopulation(
                    unevaluated,
                    1,
                    random,
                    noMutation,
                    ae::SelectionMode::Uniform
                );
        }
        catch (const std::logic_error&)
        {
            threw =
                true;
        }

        require(
            threw,
            "Unevaluated population was allowed to reproduce."
        );


        std::cout
            << "All reproduction tests passed.\n";

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