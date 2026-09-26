#include <cmath>
#include <iostream>
#include <stdexcept>

#include "alien_evolution/evaluation/Energetics.hpp"
#include "alien_evolution/evaluation/Fitness.hpp"

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
        const double tolerance = 1.0e-12
    )
    {
        return std::abs(a - b) <= tolerance;
    }

} // namespace

int main()
{
    try
    {
        // Positive energetic surplus produces equal fitness.
        ae::EnergeticConsequences positive{};
        positive.resourceAcquisition = 10.0;
        positive.maintenanceCost = 4.0;
        positive.netEnergy = 6.0;

        require(
            nearlyEqual(
                ae::calculateFitness(positive),
                6.0
            ),
            "Positive net energy produced incorrect fitness."
        );

        // Negative energetic balance cannot reproduce.
        ae::EnergeticConsequences negative{};
        negative.resourceAcquisition = 2.0;
        negative.maintenanceCost = 5.0;
        negative.netEnergy = -3.0;

        require(
            nearlyEqual(
                ae::calculateFitness(negative),
                0.0
            ),
            "Negative net energy should produce zero fitness."
        );

        // Exactly balanced energetics produces zero surplus.
        ae::EnergeticConsequences balanced{};
        balanced.resourceAcquisition = 5.0;
        balanced.maintenanceCost = 5.0;
        balanced.netEnergy = 0.0;

        require(
            nearlyEqual(
                ae::calculateFitness(balanced),
                0.0
            ),
            "Zero net energy should produce zero fitness."
        );

        std::cout
            << "All fitness tests passed.\n";

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