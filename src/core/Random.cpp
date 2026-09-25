#include "alien_evolution/core/Random.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace ae
{

    Random::Random(const std::uint64_t seed)
        : seed_(seed),
        engine_(seed)
    {}

    std::uint64_t Random::seed() const
    {
        return seed_;
    }

    std::uint64_t Random::raw()
    {
        return engine_();
    }

    double Random::uniform01()
    {
        // Convert the upper 53 random bits into a double in [0, 1).
        //
        // Doing this explicitly rather than using
        // std::uniform_real_distribution gives us tighter control over
        // reproducibility across implementations.
        constexpr double inverse53 =
            1.0 / 9007199254740992.0; // 2^53

        return static_cast<double>(raw() >> 11) * inverse53;
    }

    double Random::uniform(
        const double minimum,
        const double maximum
    )
    {
        if (maximum < minimum)
        {
            throw std::invalid_argument(
                "Random uniform range has maximum < minimum."
            );
        }

        return minimum
            + (maximum - minimum) * uniform01();
    }

    bool Random::chance(const double probability)
    {
        const double clamped =
            std::clamp(probability, 0.0, 1.0);

        return uniform01() < clamped;
    }

    double Random::normal(
        const double mean,
        const double standardDeviation
    )
    {
        if (standardDeviation < 0.0)
        {
            throw std::invalid_argument(
                "Normal standard deviation cannot be negative."
            );
        }

        if (standardDeviation == 0.0)
        {
            return mean;
        }

        if (hasSpareNormal_)
        {
            hasSpareNormal_ = false;

            return mean
                + standardDeviation * spareNormal_;
        }

        // Box-Muller transform.
        //
        // Avoid exactly zero because log(0) is undefined.
        double u1 = 0.0;

        do
        {
            u1 = uniform01();
        } while (u1 <= 0.0);

        const double u2 = uniform01();

        const double magnitude =
            std::sqrt(-2.0 * std::log(u1));

        const double angle =
            2.0 * std::numbers::pi * u2;

        const double z0 =
            magnitude * std::cos(angle);

        const double z1 =
            magnitude * std::sin(angle);

        spareNormal_ = z1;
        hasSpareNormal_ = true;

        return mean
            + standardDeviation * z0;
    }

} // namespace ae