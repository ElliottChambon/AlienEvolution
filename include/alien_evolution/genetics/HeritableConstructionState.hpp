#pragma once

#include <utility>

#include "alien_evolution/genetics/HeritableProgram.hpp"

namespace ae
{
    // Migration seam for physically heritable construction/regulation state G.
    // Unlike M1-M3 scientific metadata, this is actual organismal inherited state.
    // M4 owns only the active prototype compatibility payload; HeritableProgram
    // does not define mature HCS. Future reviewed inheritance backends may replace
    // or extend this scaffold without assuming DNA, sequences or regulatory graphs.
    class HeritableConstructionState
    {
    public:
        [[nodiscard]] static HeritableConstructionState fromPrototypeHeritableProgram(
            HeritableProgram program)
        {
            return HeritableConstructionState(std::move(program));
        }

        [[nodiscard]] const HeritableProgram& prototypeHeritableProgram() const
        {
            return prototypeProgram_;
        }

    private:
        // Existing component semantics apply; no cross-component validation,
        // target repair, new mutation opportunities or scientific metadata.
        explicit HeritableConstructionState(HeritableProgram program)
            : prototypeProgram_(std::move(program))
        {}

        HeritableProgram prototypeProgram_;
    };
} // namespace ae
