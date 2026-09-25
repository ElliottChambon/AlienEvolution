#include <cmath>
#include <iostream>
#include <stdexcept>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/development/Phenotype.hpp"
#include "alien_evolution/evolution/Organism.hpp"
#include "alien_evolution/evolution/Population.hpp"
#include "alien_evolution/genetics/Genome.hpp"
#include "alien_evolution/genetics/Mutation.hpp"

namespace
{

    void require(
        const bool condition,
        const char* message
    )
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    bool genomesEqual(
        const ae::Genome& a,
        const ae::Genome& b,
        const double tolerance = 1.0e-12
    )
    {
        return
            std::abs(a.alphaR - b.alphaR) <= tolerance
            && std::abs(a.alphaE - b.alphaE) <= tolerance
            && std::abs(a.theta - b.theta) <= tolerance
            && std::abs(a.lambda - b.lambda) <= tolerance
            && std::abs(a.beta - b.beta) <= tolerance
            && std::abs(a.growthRate - b.growthRate) <= tolerance
            && std::abs(a.metabolicCost - b.metabolicCost) <= tolerance;
    }

} // namespace

int main()
{
    try
    {
        const ae::Genome founder{};

        ae::MutationConfig variation{};
        variation.probabilityPerParameter = 1.0;
        variation.logStandardDeviation = 0.10;

        // --------------------------------------------------------
        // Test 1: requested population size is created.
        // --------------------------------------------------------

        ae::Random randomA(12345);

        ae::Population populationA(
            100,
            founder,
            randomA,
            variation
        );

        require(
            populationA.size() == 100,
            "Population size is incorrect."
        );

        // --------------------------------------------------------
        // Test 2: newborn organisms start unevaluated.
        // --------------------------------------------------------

        for (const ae::Organism& organism :
            populationA.organisms())
        {
            require(
                !organism.hasPhenotype(),
                "New organism unexpectedly has a phenotype."
            );

            require(
                !organism.hasFitness(),
                "New organism unexpectedly has fitness."
            );
        }

        // --------------------------------------------------------
        // Test 3: identical seeds produce identical populations.
        // --------------------------------------------------------

        ae::Random randomB(987654321);
        ae::Random randomC(987654321);

        ae::Population populationB(
            50,
            founder,
            randomB,
            variation
        );

        ae::Population populationC(
            50,
            founder,
            randomC,
            variation
        );

        for (std::size_t i = 0;
            i < populationB.size();
            ++i)
        {
            require(
                genomesEqual(
                    populationB.at(i).genome(),
                    populationC.at(i).genome()
                ),
                "Identical seeds produced different populations."
            );
        }

        // --------------------------------------------------------
        // Test 4: Organism stores phenotype and fitness.
        // --------------------------------------------------------

        ae::Organism organism(founder);

        require(
            !organism.hasPhenotype(),
            "Fresh organism unexpectedly has phenotype."
        );

        ae::Phenotype phenotype(10, 10);
        phenotype.setMaterial(5, 5, 1.0);

        organism.setPhenotype(
            std::move(phenotype)
        );

        require(
            organism.hasPhenotype(),
            "Organism failed to store phenotype."
        );

        organism.setFitness(0.75);

        require(
            organism.hasFitness(),
            "Organism failed to store fitness."
        );

        require(
            std::abs(organism.fitness() - 0.75)
            <= 1.0e-12,
            "Stored organism fitness is incorrect."
        );

        organism.clearEvaluation();

        require(
            !organism.hasPhenotype()
            && !organism.hasFitness(),
            "Organism evaluation state failed to clear."
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