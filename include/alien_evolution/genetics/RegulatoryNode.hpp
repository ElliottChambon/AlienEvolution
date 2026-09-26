#pragma once

#include <cstdint>

namespace ae
{

    struct RegulatoryNode
    {
        std::uint64_t id = 0;

        double initialActivity = 0.0;

        double maximumProductionRate = 1.0;

        double degradationRate = 1.0;
    };

} // namespace ae