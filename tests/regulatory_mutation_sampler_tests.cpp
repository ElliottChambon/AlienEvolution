#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/genetics/RegulatoryMutationSampler.hpp"

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

bool nearlyEqual(
    const double a,
    const double b,
    const double tolerance
)
{
    return
        std::abs(a - b)
        <= tolerance;
}

} // namespace

int main()
{
    try
    {
        // --------------------------------------------------------
        // Test 1:
        // Zero hazards produce zero realized mutations.
        // --------------------------------------------------------

        const ae::RegulatoryMutationHazards zeroHazards{};

        ae::Random zeroRandom(12345);

        const ae::RegulatoryMutationCounts zeroCounts =
            ae::sampleRegulatoryMutationCounts(
                zeroHazards,
                zeroRandom
            );

        require(
            zeroCounts.total() == 0,
            "Zero mutation hazards produced realized mutations."
        );

        // --------------------------------------------------------
        // Test 2:
        // Identical seeds produce identical mutation counts.
        // --------------------------------------------------------

        ae::RegulatoryMutationHazards deterministicHazards{};

        deterministicHazards.expectedNodeKineticEvents =
            0.5;

        deterministicHazards.expectedInteractionParameterEvents =
            1.5;

        deterministicHazards.expectedInteractionGainEvents =
            0.25;

        deterministicHazards.expectedInteractionLossEvents =
            0.75;

        deterministicHazards.expectedDuplicationEvents =
            0.1;

        deterministicHazards.expectedNodeLossEvents =
            0.2;

        ae::Random randomA(987654321);
        ae::Random randomB(987654321);

        for (std::size_t i = 0;
             i < 1000;
             ++i)
        {
            const ae::RegulatoryMutationCounts a =
                ae::sampleRegulatoryMutationCounts(
                    deterministicHazards,
                    randomA
                );

            const ae::RegulatoryMutationCounts b =
                ae::sampleRegulatoryMutationCounts(
                    deterministicHazards,
                    randomB
                );

            require(
                a.nodeKinetic
                    == b.nodeKinetic
                && a.interactionParameter
                    == b.interactionParameter
                && a.interactionGain
                    == b.interactionGain
                && a.interactionLoss
                    == b.interactionLoss
                && a.regulatoryUnitDuplication
                    == b.regulatoryUnitDuplication
                && a.regulatoryNodeLoss
                    == b.regulatoryNodeLoss,
                "Identical seeds produced different mutation counts."
            );
        }

        // --------------------------------------------------------
        // Test 3:
        // Sample mean approaches requested small Poisson mean.
        // --------------------------------------------------------

        constexpr std::size_t sampleCount =
            20000;

        ae::RegulatoryMutationHazards smallHazard{};

        smallHazard.expectedNodeKineticEvents =
            0.4;

        ae::Random smallRandom(1111);

        double smallTotal = 0.0;

        for (std::size_t i = 0;
             i < sampleCount;
             ++i)
        {
            const auto counts =
                ae::sampleRegulatoryMutationCounts(
                    smallHazard,
                    smallRandom
                );

            smallTotal +=
                static_cast<double>(
                    counts.nodeKinetic
                );
        }

        const double smallMean =
            smallTotal
            / static_cast<double>(
                sampleCount
            );

        require(
            nearlyEqual(
                smallMean,
                0.4,
                0.02
            ),
            "Small-mean Poisson sampler failed statistical mean check."
        );

        // --------------------------------------------------------
        // Test 4:
        // Large-mean sampler also approaches requested mean.
        // --------------------------------------------------------

        ae::RegulatoryMutationHazards largeHazard{};

        largeHazard.expectedNodeKineticEvents =
            12.0;

        ae::Random largeRandom(2222);

        double largeTotal = 0.0;

        for (std::size_t i = 0;
             i < sampleCount;
             ++i)
        {
            const auto counts =
                ae::sampleRegulatoryMutationCounts(
                    largeHazard,
                    largeRandom
                );

            largeTotal +=
                static_cast<double>(
                    counts.nodeKinetic
                );
        }

        const double largeMean =
            largeTotal
            / static_cast<double>(
                sampleCount
            );

        require(
            nearlyEqual(
                largeMean,
                12.0,
                0.10
            ),
            "Large-mean Poisson sampler failed statistical mean check."
        );

        // --------------------------------------------------------
        // Test 5:
        // Different mutation classes preserve their own means.
        //
        // These are numerical test inputs only, not biological
        // mutation-rate assumptions.
        // --------------------------------------------------------

        ae::RegulatoryMutationHazards mixedHazards{};

        mixedHazards.expectedNodeKineticEvents =
            0.2;

        mixedHazards.expectedInteractionParameterEvents =
            0.5;

        mixedHazards.expectedInteractionGainEvents =
            1.0;

        mixedHazards.expectedInteractionLossEvents =
            2.0;

        mixedHazards.expectedDuplicationEvents =
            0.1;

        mixedHazards.expectedNodeLossEvents =
            0.3;

        ae::Random mixedRandom(3333);

        double nodeTotal = 0.0;
        double parameterTotal = 0.0;
        double gainTotal = 0.0;
        double lossTotal = 0.0;
        double duplicationTotal = 0.0;
        double nodeLossTotal = 0.0;

        for (std::size_t i = 0;
             i < sampleCount;
             ++i)
        {
            const auto counts =
                ae::sampleRegulatoryMutationCounts(
                    mixedHazards,
                    mixedRandom
                );

            nodeTotal +=
                static_cast<double>(
                    counts.nodeKinetic
                );

            parameterTotal +=
                static_cast<double>(
                    counts.interactionParameter
                );

            gainTotal +=
                static_cast<double>(
                    counts.interactionGain
                );

            lossTotal +=
                static_cast<double>(
                    counts.interactionLoss
                );

            duplicationTotal +=
                static_cast<double>(
                    counts.regulatoryUnitDuplication
                );

            nodeLossTotal +=
                static_cast<double>(
                    counts.regulatoryNodeLoss
                );
        }

        const double denominator =
            static_cast<double>(
                sampleCount
            );

        require(
            nearlyEqual(
                nodeTotal / denominator,
                0.2,
                0.02
            ),
            "Node kinetic event mean is incorrect."
        );

        require(
            nearlyEqual(
                parameterTotal / denominator,
                0.5,
                0.03
            ),
            "Interaction parameter event mean is incorrect."
        );

        require(
            nearlyEqual(
                gainTotal / denominator,
                1.0,
                0.04
            ),
            "Interaction gain event mean is incorrect."
        );

        require(
            nearlyEqual(
                lossTotal / denominator,
                2.0,
                0.05
            ),
            "Interaction loss event mean is incorrect."
        );

        require(
            nearlyEqual(
                duplicationTotal / denominator,
                0.1,
                0.015
            ),
            "Duplication event mean is incorrect."
        );

        require(
            nearlyEqual(
                nodeLossTotal / denominator,
                0.3,
                0.02
            ),
            "Node-loss event mean is incorrect."
        );

        std::cout
            << "All regulatory mutation sampler tests passed.\n";

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