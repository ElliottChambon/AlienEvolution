#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/evolution/Organism.hpp"
#include "alien_evolution/evolution/Population.hpp"
#include "alien_evolution/evolution/Reproduction.hpp"
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
        ae::MutationConfig noMutation{};
        noMutation.probabilityPerParameter = 0.0;

        // --------------------------------------------------------
        // Test 1:
        // A single viable parent produces exact copies when
        // mutation is disabled.
        // --------------------------------------------------------

        ae::Genome parentGenome{};
        parentGenome.alphaR = 2.0;

        ae::Organism parent(parentGenome);
        parent.setFitness(10.0);

        std::vector<ae::Organism> oneParent;
        oneParent.push_back(parent);

        ae::Population parentPopulation(
            std::move(oneParent)
        );

        ae::Random randomA(12345);

        const ae::Population children =
            ae::reproducePopulation(
                parentPopulation,
                100,
                randomA,
                noMutation
            );

        require(
            children.size() == 100,
            "Incorrect offspring population size."
        );

        for (const ae::Organism& child :
            children.organisms())
        {
            require(
                genomesEqual(
                    child.genome(),
                    parentGenome
                ),
                "Inheritance changed genome with mutation disabled."
            );

            require(
                !child.hasFitness(),
                "New offspring unexpectedly has fitness."
            );

            require(
                !child.hasPhenotype(),
                "New offspring unexpectedly has phenotype."
            );
        }

        // --------------------------------------------------------
        // Test 2:
        // Zero-fitness organisms cannot reproduce when a viable
        // organism exists.
        // --------------------------------------------------------

        ae::Genome badGenome{};
        badGenome.alphaR = 100.0;

        ae::Genome goodGenome{};
        goodGenome.alphaR = 3.0;

        ae::Organism bad(badGenome);
        ae::Organism good(goodGenome);

        bad.setFitness(0.0);
        good.setFitness(1.0);

        std::vector<ae::Organism> mixedParents;

        mixedParents.push_back(bad);
        mixedParents.push_back(good);

        ae::Population mixedPopulation(
            std::move(mixedParents)
        );

        ae::Random randomB(67890);

        const ae::Population selectedChildren =
            ae::reproducePopulation(
                mixedPopulation,
                100,
                randomB,
                noMutation
            );

        for (const ae::Organism& child :
            selectedChildren.organisms())
        {
            require(
                genomesEqual(
                    child.genome(),
                    goodGenome
                ),
                "Zero-fitness organism reproduced."
            );
        }

        // --------------------------------------------------------
        // Test 3:
        // Completely non-viable population becomes extinct.
        // --------------------------------------------------------

        ae::Organism deadA(parentGenome);
        ae::Organism deadB(parentGenome);

        deadA.setFitness(0.0);
        deadB.setFitness(0.0);

        std::vector<ae::Organism> deadParents;

        deadParents.push_back(deadA);
        deadParents.push_back(deadB);

        ae::Population deadPopulation(
            std::move(deadParents)
        );

        ae::Random randomC(42);

        const ae::Population extinct =
            ae::reproducePopulation(
                deadPopulation,
                100,
                randomC,
                noMutation
            );

        require(
            extinct.empty(),
            "Zero-fitness population failed to go extinct."
        );

        // --------------------------------------------------------
        // Test 4:
        // Same seed produces identical offspring populations.
        // --------------------------------------------------------

        ae::MutationConfig mutation{};
        mutation.probabilityPerParameter = 0.5;
        mutation.logStandardDeviation = 0.1;

        ae::Organism reproducibleParent(parentGenome);
        reproducibleParent.setFitness(1.0);

        std::vector<ae::Organism> parentsD;
        std::vector<ae::Organism> parentsE;

        parentsD.push_back(reproducibleParent);
        parentsE.push_back(reproducibleParent);

        ae::Population populationD(
            std::move(parentsD)
        );

        ae::Population populationE(
            std::move(parentsE)
        );

        ae::Random randomD(999);
        ae::Random randomE(999);

        const ae::Population childrenD =
            ae::reproducePopulation(
                populationD,
                50,
                randomD,
                mutation
            );

        const ae::Population childrenE =
            ae::reproducePopulation(
                populationE,
                50,
                randomE,
                mutation
            );

        require(
            childrenD.size() == childrenE.size(),
            "Deterministic reproduction changed population size."
        );

        for (std::size_t i = 0;
            i < childrenD.size();
            ++i)
        {
            require(
                genomesEqual(
                    childrenD.at(i).genome(),
                    childrenE.at(i).genome()
                ),
                "Identical seeds produced different offspring."
            );
        }

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