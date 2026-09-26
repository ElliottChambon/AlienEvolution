#pragma once

#include <cstddef>

#include "alien_evolution/genetics/RegulatoryProgram.hpp"

namespace ae
{

    // Rates are expected mutation events per eligible target
    // per replication.
    //
    // V0.2 deliberately supplies no biological default rates.
    // All values therefore default to zero.
    //
    // Earth-calibrated models, experimental null models, or future
    // mechanistic replication systems must explicitly provide them.
    struct RegulatoryMutationRateModel
    {
        double nodeKineticPerNode = 0.0;

        double interactionParameterPerInteraction = 0.0;

        double interactionGainPerAbsentPair = 0.0;

        double interactionLossPerInteraction = 0.0;

        double regulatoryUnitDuplicationPerNode = 0.0;

        double regulatoryNodeLossPerDeletableNode = 0.0;
    };

    // Expected mutation counts for one replication of the
    // current regulatory program.
    //
    // These are hazards / expectations, not realized events.
    struct RegulatoryMutationHazards
    {
        std::size_t eligibleNodeKineticTargets = 0;

        std::size_t eligibleInteractionParameterTargets = 0;

        std::size_t eligibleInteractionGainTargets = 0;

        std::size_t eligibleInteractionLossTargets = 0;

        std::size_t eligibleDuplicationTargets = 0;

        std::size_t eligibleNodeLossTargets = 0;


        double expectedNodeKineticEvents = 0.0;

        double expectedInteractionParameterEvents = 0.0;

        double expectedInteractionGainEvents = 0.0;

        double expectedInteractionLossEvents = 0.0;

        double expectedDuplicationEvents = 0.0;

        double expectedNodeLossEvents = 0.0;


        double totalExpectedEvents = 0.0;
    };

    [[nodiscard]]
    RegulatoryMutationHazards computeRegulatoryMutationHazards(
        const RegulatoryProgram& program,
        const RegulatoryMutationRateModel& rates
    );

} // namespace ae