#pragma once

#include <cstdint>

namespace ae
{

    struct RegulatoryInteraction
    {
        std::uint64_t sourceNodeId = 0;
        std::uint64_t targetNodeId = 0;

        // Positive = activation
        // Negative = repression
        double strength = 1.0;

        // Regulator activity at approximately half-maximal response.
        double halfSaturation = 0.5;

        // Hill-like cooperativity parameter.
        double cooperativity = 1.0;
    };

} // namespace ae