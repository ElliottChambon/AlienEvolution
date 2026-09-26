#pragma once

#include <cstdint>

namespace ae
{

    struct RegulatoryInteraction
    {
        std::uint64_t sourceNodeId = 0;
        std::uint64_t targetNodeId = 0;

        // Fold change in target production when the regulator
        // is strongly active.
        //
        // foldChange > 1 : activation
        // foldChange < 1 : repression
        // foldChange = 1 : no regulatory effect
        double foldChange = 1.0;

        // Activity scale at which the interaction is approximately
        // half engaged.
        double halfSaturation = 0.5;

        // Hill-like cooperativity / nonlinear response parameter.
        double cooperativity = 1.0;
    };

} // namespace ae