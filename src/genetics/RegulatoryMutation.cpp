#include "alien_evolution/genetics/RegulatoryMutation.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ae
{

    namespace
    {

        void requirePositiveFinite(
            const double value,
            const char* message
        )
        {
            if (
                !std::isfinite(value)
                || value <= 0.0
                )
            {
                throw std::invalid_argument(
                    message
                );
            }
        }

    } // namespace

    RegulatoryProgram changeNodeKinetics(
        const RegulatoryProgram& program,
        const std::uint64_t nodeId,
        const NodeKineticChange& change
    )
    {
        requirePositiveFinite(
            change.basalProductionMultiplier,
            "Basal production multiplier must be finite and positive."
        );

        requirePositiveFinite(
            change.degradationMultiplier,
            "Degradation multiplier must be finite and positive."
        );

        if (!program.containsNode(nodeId))
        {
            throw std::out_of_range(
                "Cannot mutate nonexistent regulatory node."
            );
        }

        std::vector<RegulatoryNode> nodes =
            program.nodes();

        for (RegulatoryNode& node : nodes)
        {
            if (node.id != nodeId)
            {
                continue;
            }

            node.basalProductionRate *=
                change.basalProductionMultiplier;

            node.degradationRate *=
                change.degradationMultiplier;

            if (
                !std::isfinite(
                    node.basalProductionRate
                )
                || !std::isfinite(
                    node.degradationRate
                )
                )
            {
                throw std::overflow_error(
                    "Node kinetic mutation produced non-finite parameter."
                );
            }

            break;
        }

        return RegulatoryProgram(
            std::move(nodes),
            program.interactions()
        );
    }

    RegulatoryProgram changeInteractionParameters(
        const RegulatoryProgram& program,
        const std::size_t interactionIndex,
        const InteractionParameterChange& change
    )
    {
        requirePositiveFinite(
            change.foldChangeMultiplier,
            "Fold-change multiplier must be finite and positive."
        );

        requirePositiveFinite(
            change.halfSaturationMultiplier,
            "Half-saturation multiplier must be finite and positive."
        );

        requirePositiveFinite(
            change.cooperativityMultiplier,
            "Cooperativity multiplier must be finite and positive."
        );

        if (
            interactionIndex
            >= program.interactionCount()
            )
        {
            throw std::out_of_range(
                "Cannot mutate nonexistent regulatory interaction."
            );
        }

        std::vector<RegulatoryInteraction> interactions =
            program.interactions();

        RegulatoryInteraction& interaction =
            interactions[interactionIndex];

        interaction.foldChange *=
            change.foldChangeMultiplier;

        interaction.halfSaturation *=
            change.halfSaturationMultiplier;

        interaction.cooperativity *=
            change.cooperativityMultiplier;

        if (
            !std::isfinite(
                interaction.foldChange
            )
            || !std::isfinite(
                interaction.halfSaturation
            )
            || !std::isfinite(
                interaction.cooperativity
            )
            )
        {
            throw std::overflow_error(
                "Interaction mutation produced non-finite parameter."
            );
        }

        return RegulatoryProgram(
            program.nodes(),
            std::move(interactions)
        );
    }

    RegulatoryProgram addRegulatoryInteraction(
        const RegulatoryProgram& program,
        const RegulatoryInteraction& interaction
    )
    {
        if (
            !program.containsNode(
                interaction.sourceNodeId
            )
            || !program.containsNode(
                interaction.targetNodeId
            )
            )
        {
            throw std::invalid_argument(
                "New regulatory interaction references nonexistent node."
            );
        }

        for (
            const RegulatoryInteraction& existing :
            program.interactions()
            )
        {
            if (
                existing.sourceNodeId
                == interaction.sourceNodeId
                && existing.targetNodeId
                == interaction.targetNodeId
                )
            {
                throw std::invalid_argument(
                    "Duplicate regulatory interaction is not allowed."
                );
            }
        }

        std::vector<RegulatoryInteraction> interactions =
            program.interactions();

        interactions.push_back(
            interaction
        );

        return RegulatoryProgram(
            program.nodes(),
            std::move(interactions)
        );
    }

    RegulatoryProgram removeRegulatoryInteraction(
        const RegulatoryProgram& program,
        const std::size_t interactionIndex
    )
    {
        if (
            interactionIndex
            >= program.interactionCount()
            )
        {
            throw std::out_of_range(
                "Cannot remove nonexistent regulatory interaction."
            );
        }

        std::vector<RegulatoryInteraction> interactions =
            program.interactions();

        interactions.erase(
            interactions.begin()
            + static_cast<std::ptrdiff_t>(
                interactionIndex
                )
        );

        return RegulatoryProgram(
            program.nodes(),
            std::move(interactions)
        );
    }

} // namespace ae