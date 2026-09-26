#pragma once

#include <cstdint>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/genetics/RegulatoryProgram.hpp"

namespace ae
{

    struct RegulatoryInteractionTarget
    {
        std::uint64_t sourceNodeId = 0;
        std::uint64_t targetNodeId = 0;
    };

    // ------------------------------------------------------------
    // V0.2 uniform-target null model.
    //
    // These functions select uniformly among the currently eligible
    // abstract regulatory targets.
    //
    // Uniformity here is a computational null hypothesis.
    // It is NOT a claim that real molecular mutation probabilities
    // are uniform across genes, regulatory elements, or genomes.
    // ------------------------------------------------------------

    [[nodiscard]]
    std::uint64_t selectUniformNodeKineticTarget(
        const RegulatoryProgram& program,
        Random& random
    );

    [[nodiscard]]
    RegulatoryInteractionTarget
        selectUniformInteractionParameterTarget(
            const RegulatoryProgram& program,
            Random& random
        );

    [[nodiscard]]
    RegulatoryInteractionTarget
        selectUniformInteractionGainTarget(
            const RegulatoryProgram& program,
            Random& random
        );

    [[nodiscard]]
    RegulatoryInteractionTarget
        selectUniformInteractionLossTarget(
            const RegulatoryProgram& program,
            Random& random
        );

    [[nodiscard]]
    std::uint64_t selectUniformDuplicationTarget(
        const RegulatoryProgram& program,
        Random& random
    );

    [[nodiscard]]
    std::uint64_t selectUniformNodeLossTarget(
        const RegulatoryProgram& program,
        Random& random
    );

} // namespace ae