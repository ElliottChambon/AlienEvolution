#include "alien_evolution/genetics/RegulatoryProgram.hpp"

#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace ae
{

    RegulatoryProgram::RegulatoryProgram(
        std::vector<RegulatoryNode> nodes,
        std::vector<RegulatoryInteraction> interactions
    )
        : nodes_(std::move(nodes)),
        interactions_(std::move(interactions))
    {
        validate();
    }

    const std::vector<RegulatoryNode>&
        RegulatoryProgram::nodes() const
    {
        return nodes_;
    }

    const std::vector<RegulatoryInteraction>&
        RegulatoryProgram::interactions() const
    {
        return interactions_;
    }

    std::size_t RegulatoryProgram::nodeCount() const
    {
        return nodes_.size();
    }

    std::size_t RegulatoryProgram::interactionCount() const
    {
        return interactions_.size();
    }

    bool RegulatoryProgram::containsNode(
        const std::uint64_t nodeId
    ) const
    {
        for (const RegulatoryNode& node :
            nodes_)
        {
            if (node.id == nodeId)
            {
                return true;
            }
        }

        return false;
    }

    const RegulatoryNode& RegulatoryProgram::node(
        const std::uint64_t nodeId
    ) const
    {
        for (const RegulatoryNode& node :
            nodes_)
        {
            if (node.id == nodeId)
            {
                return node;
            }
        }

        throw std::out_of_range(
            "Regulatory node ID does not exist."
        );
    }

    void RegulatoryProgram::validate() const
    {
        if (nodes_.empty())
        {
            throw std::invalid_argument(
                "Regulatory program must contain at least one node."
            );
        }

        std::unordered_set<std::uint64_t> nodeIds;

        for (const RegulatoryNode& node :
            nodes_)
        {
            if (!nodeIds.insert(node.id).second)
            {
                throw std::invalid_argument(
                    "Regulatory node IDs must be unique."
                );
            }

            if (
                !std::isfinite(node.initialActivity)
                || node.initialActivity < 0.0
                )
            {
                throw std::invalid_argument(
                    "Initial regulatory activity must be finite and nonnegative."
                );
            }

            if (
                !std::isfinite(
                    node.maximumProductionRate
                )
                || node.maximumProductionRate < 0.0
                )
            {
                throw std::invalid_argument(
                    "Maximum production rate must be finite and nonnegative."
                );
            }

            if (
                !std::isfinite(
                    node.degradationRate
                )
                || node.degradationRate <= 0.0
                )
            {
                throw std::invalid_argument(
                    "Degradation rate must be finite and positive."
                );
            }
        }

        for (const RegulatoryInteraction& interaction :
            interactions_)
        {
            if (
                !containsNode(
                    interaction.sourceNodeId
                )
                )
            {
                throw std::invalid_argument(
                    "Regulatory interaction source node does not exist."
                );
            }

            if (
                !containsNode(
                    interaction.targetNodeId
                )
                )
            {
                throw std::invalid_argument(
                    "Regulatory interaction target node does not exist."
                );
            }

            if (
                !std::isfinite(
                    interaction.strength
                )
                )
            {
                throw std::invalid_argument(
                    "Regulatory interaction strength must be finite."
                );
            }

            if (
                !std::isfinite(
                    interaction.halfSaturation
                )
                || interaction.halfSaturation <= 0.0
                )
            {
                throw std::invalid_argument(
                    "Interaction half-saturation must be finite and positive."
                );
            }

            if (
                !std::isfinite(
                    interaction.cooperativity
                )
                || interaction.cooperativity <= 0.0
                )
            {
                throw std::invalid_argument(
                    "Interaction cooperativity must be finite and positive."
                );
            }
        }
    }

} // namespace ae