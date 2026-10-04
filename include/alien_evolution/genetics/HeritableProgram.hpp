#pragma once

#include <utility>

#include "alien_evolution/genetics/RegulatoryProgram.hpp"
#include "alien_evolution/genetics/SensoryProgram.hpp"

namespace ae
{
    // Joint ownership/inheritance of scientifically distinct components.
    // Cross-component couplings are checked by the runtime input adapter.
    class HeritableProgram
    {
    public:
        HeritableProgram(
            RegulatoryProgram regulatory,
            SensoryProgram sensory = SensoryProgram{}
        )
            : regulatory_(std::move(regulatory)), sensory_(std::move(sensory))
        {}

        [[nodiscard]] const RegulatoryProgram& regulatoryProgram() const
        {
            return regulatory_;
        }

        [[nodiscard]] const SensoryProgram& sensoryProgram() const
        {
            return sensory_;
        }

    private:
        RegulatoryProgram regulatory_;
        SensoryProgram sensory_;
    };
} // namespace ae
