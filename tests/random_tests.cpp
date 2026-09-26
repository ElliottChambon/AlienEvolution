#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>

#include "alien_evolution/core/Random.hpp"

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

} // namespace

int main()
{
    try
    {
        // --------------------------------------------------------
        // Test 1:
        // Same seed produces identical uniform sequence.
        // --------------------------------------------------------

        ae::Random uniformA(
            12345
        );

        ae::Random uniformB(
            12345
        );

        for (
            std::size_t i = 0;
            i < 10000;
            ++i
            )
        {
            require(
                uniformA.uniform01()
                == uniformB.uniform01(),
                "Identical seeds produced different uniform sequence."
            );
        }


        // --------------------------------------------------------
        // Test 2:
        // uniform01 remains inside [0,1).
        // --------------------------------------------------------

        ae::Random bounded(
            999
        );

        for (
            std::size_t i = 0;
            i < 100000;
            ++i
            )
        {
            const double value =
                bounded.uniform01();

            require(
                value >= 0.0
                && value < 1.0,
                "uniform01 produced value outside [0,1)."
            );
        }


        // --------------------------------------------------------
        // Test 3:
        // Same seed produces identical normal sequence.
        // --------------------------------------------------------

        ae::Random normalA(
            777
        );

        ae::Random normalB(
            777
        );

        for (
            std::size_t i = 0;
            i < 10000;
            ++i
            )
        {
            require(
                normalA.normal(
                    0.0,
                    1.0
                )
                ==
                normalB.normal(
                    0.0,
                    1.0
                ),
                "Identical seeds produced different normal sequence."
            );
        }


        // --------------------------------------------------------
        // Test 4:
        // Normal sampler approximately reproduces requested mean
        // and variance.
        // --------------------------------------------------------

        constexpr std::size_t sampleCount =
            50000;

        ae::Random statistical(
            4444
        );

        double sum =
            0.0;

        double sumSquares =
            0.0;

        constexpr double requestedMean =
            3.0;

        constexpr double requestedStdDev =
            2.0;

        for (
            std::size_t i = 0;
            i < sampleCount;
            ++i
            )
        {
            const double value =
                statistical.normal(
                    requestedMean,
                    requestedStdDev
                );

            sum +=
                value;

            sumSquares +=
                value * value;
        }

        const double denominator =
            static_cast<double>(
                sampleCount
                );

        const double sampleMean =
            sum / denominator;

        const double sampleVariance =
            sumSquares / denominator
            - sampleMean * sampleMean;

        require(
            std::abs(
                sampleMean
                - requestedMean
            )
            < 0.04,
            "Normal sampler mean is outside tolerance."
        );

        require(
            std::abs(
                sampleVariance
                - requestedStdDev
                * requestedStdDev
            )
            < 0.10,
            "Normal sampler variance is outside tolerance."
        );


        std::cout
            << "All random tests passed.\n";

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