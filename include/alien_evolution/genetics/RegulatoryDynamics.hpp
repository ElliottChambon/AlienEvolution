#pragma once

#include <cstddef>
#include <vector>

#include "alien_evolution/genetics/RegulatoryProgram.hpp"

namespace ae
{

    using RegulatoryState =
        std::vector<double>;

    struct RegulatoryStateClamp
    {
        std::size_t index = 0;
        double value = 0.0;
    };

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

        // RK4 integration with externally imposed state components.
        //
        // Clamped components are held at their specified values during
        // every RK4 derivative evaluation and at the final state.
        //
        // Their intrinsic regulatory derivative is not evaluated because
        // the external boundary condition, rather than their internal
        // kinetics, determines their state.
        [[nodiscard]]
        static RegulatoryState stepRK4(
            const RegulatoryProgram& program,
            const RegulatoryState& state,
            double timeStep,
            const std::vector<RegulatoryStateClamp>& clamps
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