#include <cmath>
#include <iostream>
#include <stdexcept>

#include "alien_evolution/core/Random.hpp"
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

    bool allPositive(const ae::Genome& genome)
    {
        return
            genome.alphaR > 0.0
            && genome.alphaE > 0.0
            && genome.theta > 0.0
            && genome.lambda > 0.0
            && genome.beta > 0.0
            && genome.growthRate > 0.0
            && genome.metabolicCost > 0.0;
    }

} // namespace

int main()
{
    try
    {
        // --------------------------------------------------------
        // Test 1: identical seeds produce identical RNG sequences.
        // --------------------------------------------------------

        ae::Random randomA(12345);
        ae::Random randomB(12345);

        for (int i = 0; i < 100; ++i)
        {
            require(
                randomA.raw() == randomB.raw(),
                "Identical RNG seeds produced different sequences."
            );
        }

        // --------------------------------------------------------
        // Test 2: zero mutation probability preserves the genome.
        // --------------------------------------------------------

        const ae::Genome parent{};

        ae::MutationConfig noMutation{};
        noMutation.probabilityPerParameter = 0.0;

        ae::Random randomC(1);

        const ae::Genome unchanged =
            ae::mutateGenome(
                parent,
                randomC,
                noMutation
            );

        require(
            genomesEqual(parent, unchanged),
            "Zero mutation probability changed the genome."
        );

        // --------------------------------------------------------
        // Test 3: guaranteed mutation changes the genome.
        // --------------------------------------------------------

        ae::MutationConfig fullMutation{};
        fullMutation.probabilityPerParameter = 1.0;
        fullMutation.logStandardDeviation = 0.10;

        ae::Random randomD(42);

        const ae::Genome mutated =
            ae::mutateGenome(
                parent,
                randomD,
                fullMutation
            );

        require(
            !genomesEqual(parent, mutated),
            "Guaranteed mutation failed to change the genome."
        );

        // --------------------------------------------------------
        // Test 4: positive-valued V0.1 parameters remain positive.
        // --------------------------------------------------------

        require(
            allPositive(mutated),
            "Mutation produced a non-positive genome parameter."
        );

        // --------------------------------------------------------
        // Test 5: mutation is deterministic for the same seed.
        // --------------------------------------------------------

        ae::Random randomE(987654321);
        ae::Random randomF(987654321);

        const ae::Genome childA =
            ae::mutateGenome(
                parent,
                randomE,
                fullMutation
            );

        const ae::Genome childB =
            ae::mutateGenome(
                parent,
                randomF,
                fullMutation
            );

        require(
            genomesEqual(childA, childB),
            "Mutation was not reproducible with the same seed."
        );

        std::cout
            << "All random and mutation tests passed.\n";

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