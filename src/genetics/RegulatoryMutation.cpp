#include "alien_evolution/genetics/RegulatoryMutation.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
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

        bool interactionExists(
            const std::vector<RegulatoryInteraction>& interactions,
            const std::uint64_t sourceNodeId,
            const std::uint64_t targetNodeId
        )
        {
            for (const RegulatoryInteraction& interaction :
                interactions)
            {
                if (
                    interaction.sourceNodeId == sourceNodeId
                    && interaction.targetNodeId == targetNodeId
                    )
                {
                    return true;
                }
            }

            return false;
        }

        void addInteractionIfAbsent(
            std::vector<RegulatoryInteraction>& interactions,
            const RegulatoryInteraction& interaction
        )
        {
            if (
                !interactionExists(
                    interactions,
                    interaction.sourceNodeId,
                    interaction.targetNodeId
                )
                )
            {
                interactions.push_back(
                    interaction
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

        if (
            interactionExists(
                program.interactions(),
                interaction.sourceNodeId,
                interaction.targetNodeId
            )
            )
        {
            throw std::invalid_argument(
                "Duplicate regulatory interaction is not allowed."
            );
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

    RegulatoryProgram duplicateRegulatoryUnit(
        const RegulatoryProgram& program,
        const std::uint64_t sourceNodeId,
        const std::uint64_t newNodeId
    )
    {
        if (!program.containsNode(sourceNodeId))
        {
            throw std::out_of_range(
                "Cannot duplicate nonexistent regulatory node."
            );
        }

        if (program.containsNode(newNodeId))
        {
            throw std::invalid_argument(
                "New regulatory node ID is already in use."
            );
        }

        std::vector<RegulatoryNode> nodes =
            program.nodes();

        const RegulatoryNode& sourceNode =
            program.node(sourceNodeId);

        RegulatoryNode duplicate =
            sourceNode;

        duplicate.id =
            newNodeId;

        nodes.push_back(
            duplicate
        );

        std::vector<RegulatoryInteraction> interactions =
            program.interactions();

        const std::vector<RegulatoryInteraction>
            originalInteractions =
            program.interactions();

        for (const RegulatoryInteraction& interaction :
            originalInteractions)
        {
            const bool sourceIsDuplicated =
                interaction.sourceNodeId
                == sourceNodeId;

            const bool targetIsDuplicated =
                interaction.targetNodeId
                == sourceNodeId;

            // The duplicate initially receives the same incoming
            // regulation as its ancestral unit.
            if (targetIsDuplicated)
            {
                RegulatoryInteraction copiedIncoming =
                    interaction;

                copiedIncoming.targetNodeId =
                    newNodeId;

                addInteractionIfAbsent(
                    interactions,
                    copiedIncoming
                );
            }

            // The duplicate initially regulates the same downstream
            // targets as its ancestral unit.
            if (sourceIsDuplicated)
            {
                RegulatoryInteraction copiedOutgoing =
                    interaction;

                copiedOutgoing.sourceNodeId =
                    newNodeId;

                addInteractionIfAbsent(
                    interactions,
                    copiedOutgoing
                );
            }

            // If the ancestral node regulates itself, duplication of
            // the entire regulatory unit creates the corresponding
            // duplicate self-regulatory relationship as well.
            if (
                sourceIsDuplicated
                && targetIsDuplicated
                )
            {
                RegulatoryInteraction duplicateSelfInteraction =
                    interaction;

                duplicateSelfInteraction.sourceNodeId =
                    newNodeId;

                duplicateSelfInteraction.targetNodeId =
                    newNodeId;

                addInteractionIfAbsent(
                    interactions,
                    duplicateSelfInteraction
                );
            }
        }

        return RegulatoryProgram(
            std::move(nodes),
            std::move(interactions)
        );
    }

    RegulatoryProgram removeRegulatoryNode(
        const RegulatoryProgram& program,
        const std::uint64_t nodeId
    )
    {
        if (!program.containsNode(nodeId))
        {
            throw std::out_of_range(
                "Cannot remove nonexistent regulatory node."
            );
        }

        if (program.nodeCount() <= 1)
        {
            throw std::logic_error(
                "Cannot remove the final regulatory node."
            );
        }

        std::vector<RegulatoryNode> nodes =
            program.nodes();

        nodes.erase(
            std::remove_if(
                nodes.begin(),
                nodes.end(),
                [nodeId](const RegulatoryNode& node)
                {
                    return node.id == nodeId;
                }
            ),
            nodes.end()
        );

        std::vector<RegulatoryInteraction> interactions =
            program.interactions();

        interactions.erase(
            std::remove_if(
                interactions.begin(),
                interactions.end(),
                [nodeId](
                    const RegulatoryInteraction& interaction
                    )
                {
                    return
                        interaction.sourceNodeId
                        == nodeId
                        || interaction.targetNodeId
                        == nodeId;
                }
            ),
            interactions.end()
        );

        return RegulatoryProgram(
            std::move(nodes),
            std::move(interactions)
        );
    }

} // namespace ae