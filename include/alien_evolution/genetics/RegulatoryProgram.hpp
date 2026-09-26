#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "alien_evolution/genetics/RegulatoryInteraction.hpp"
#include "alien_evolution/genetics/RegulatoryNode.hpp"

namespace ae
{

    class RegulatoryProgram
    {
    public:
        RegulatoryProgram(
            std::vector<RegulatoryNode> nodes,
            std::vector<RegulatoryInteraction> interactions
        );

        [[nodiscard]]
        const std::vector<RegulatoryNode>& nodes() const;

        [[nodiscard]]
        const std::vector<RegulatoryInteraction>& interactions() const;

        [[nodiscard]]
        std::size_t nodeCount() const;

        [[nodiscard]]
        std::size_t interactionCount() const;

        [[nodiscard]]
        bool containsNode(
            std::uint64_t nodeId
        ) const;

        [[nodiscard]]
        const RegulatoryNode& node(
            std::uint64_t nodeId
        ) const;

    private:
        std::vector<RegulatoryNode> nodes_;
        std::vector<RegulatoryInteraction> interactions_;

        void validate() const;
    };

} // namespace ae