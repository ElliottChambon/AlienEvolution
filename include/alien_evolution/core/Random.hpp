#pragma once

#include <cstdint>
#include <random>

namespace ae
{

    class Random
    {
    public:
        explicit Random(std::uint64_t seed);

        [[nodiscard]] std::uint64_t seed() const;

        [[nodiscard]] std::uint64_t raw();

        [[nodiscard]] double uniform01();

        [[nodiscard]] double uniform(
            double minimum,
            double maximum
        );

        [[nodiscard]] bool chance(
            double probability
        );

        [[nodiscard]] double normal(
            double mean,
            double standardDeviation
        );

    private:
        std::uint64_t seed_;
        std::mt19937_64 engine_;

        bool hasSpareNormal_ = false;
        double spareNormal_ = 0.0;
    };

} // namespace ae