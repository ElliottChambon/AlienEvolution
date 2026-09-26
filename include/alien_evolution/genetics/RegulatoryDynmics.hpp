#pragma once

#include <vector>

#include "alien_evolution/genetics/RegulatoryProgram.hpp"

namespace ae
{

    using RegulatoryState =
        std::vector<double>;

    class RegulatoryDynamics
    {
    public:
        [[nodiscard]]
        static double shiftedHill(
            double activity,
            double halfSaturation,
            double cooperativity,
            double foldChange
        );

        [[nodiscard]]
        static RegulatoryState initialState(
            const RegulatoryProgram& program
        );

        [[nodiscard]]
        static RegulatoryState derivatives(
            const RegulatoryProgram& program,
            const RegulatoryState& state
        );

        [[nodiscard]]
        static RegulatoryState stepRK4(
            const RegulatoryProgram& program,
            const RegulatoryState& state,
            double timeStep
        );

        [[nodiscard]]
        static RegulatoryState simulate(
            const RegulatoryProgram& program,
            RegulatoryState state,
            double duration,
            double timeStep
        );
    };

} // namespace ae