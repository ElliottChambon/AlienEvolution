#pragma once

#include <cstdint>

namespace ae
{

    struct RegulatoryNode
    {
        std::uint64_t id = 0;

        // Regulatory activity present at the beginning of development.
        double initialActivity = 0.0;

        // Basal production rate before regulatory multipliers are applied.
        double basalProductionRate = 1.0;

        // First-order degradation / relaxation rate.
        double degradationRate = 1.0;
    };

} // namespace ae