#include "alien_evolution/genetics/RegulatoryMutationTargetSelector.hpp"

#include <cstddef>
#include <limits>
#include <stdexcept>

namespace ae
{

    namespace
    {

        std::size_t selectIndex(
            const std::size_t count,
            Random& random
        )
        {
            if (count == 0)
            {
                throw std::logic_error(
                    "Cannot select from an empty target set."
                );
            }

            const double scaled =
                random.uniform01()
                * static_cast<double>(count);

            std::size_t index =
                static_cast<std::size_t>(scaled);

            // Defensive guard in case the RNG implementation or floating
            // point rounding ever returns the upper endpoint.
            if (index >= count)
            {
                index = count - 1;
            }

            return index;
        }

        bool hasInteraction(
            const RegulatoryProgram& program,
            const std::uint64_t sourceNodeId,
            const std::uint64_t targetNodeId
        )
        {
            for (const RegulatoryInteraction& interaction :
                program.interactions())
            {
                if (
                    interaction.sourceNodeId
                    == sourceNodeId
                    && interaction.targetNodeId
                    == targetNodeId
                    )
                {
                    return true;
                }
            }

            return false;
        }

        RegulatoryInteractionTarget
            selectExistingInteractionTarget(
                const RegulatoryProgram& program,
                Random& random
            )
        {
            if (program.interactionCount() == 0)
            {
                throw std::logic_error(
                    "Regulatory program has no existing interaction target."
                );
            }

            const std::size_t index =
                selectIndex(
                    program.interactionCount(),
                    random
                );

            const RegulatoryInteraction& interaction =
                program.interactions()[index];

            return {
                interaction.sourceNodeId,
                interaction.targetNodeId
            };
        }

    } // namespace

    std::uint64_t selectUniformNodeKineticTarget(
        const RegulatoryProgram& program,
        Random& random
    )
    {
        const std::size_t index =
            selectIndex(
                program.nodeCount(),
                random
            );

        return program.nodes()[index].id;
    }

    RegulatoryInteractionTarget
        selectUniformInteractionParameterTarget(
            const RegulatoryProgram& program,
            Random& random
        )
    {
        return selectExistingInteractionTarget(
            program,
            random
        );
    }

    RegulatoryInteractionTarget
        selectUniformInteractionGainTarget(
            const RegulatoryProgram& program,
            Random& random
        )
    {
        const std::size_t nodeCount =
            program.nodeCount();

        if (
            nodeCount
        > std::numeric_limits<std::size_t>::max()
            / nodeCount
            )
        {
            throw std::overflow_error(
                "Regulatory network is too large to count possible interactions."
            );
        }

        const std::size_t possibleCount =
            nodeCount * nodeCount;

        const std::size_t existingCount =
            program.interactionCount();

        if (existingCount >= possibleCount)
        {
            throw std::logic_error(
                "Regulatory program has no absent interaction target."
            );
        }

        const std::size_t absentCount =
            possibleCount - existingCount;

        // --------------------------------------------------------
        // Sparse-network path.
        //
        // When at least half of all possible relationships are absent,
        // rejection sampling succeeds with probability >= 0.5 on each
        // attempt and avoids allocating/enumerating an O(N^2) list.
        // --------------------------------------------------------

        if (absentCount >= existingCount)
        {
            while (true)
            {
                const std::size_t sourceIndex =
                    selectIndex(
                        nodeCount,
                        random
                    );

                const std::size_t targetIndex =
                    selectIndex(
                        nodeCount,
                        random
                    );

                const std::uint64_t sourceNodeId =
                    program.nodes()[sourceIndex].id;

                const std::uint64_t targetNodeId =
                    program.nodes()[targetIndex].id;

                if (
                    !hasInteraction(
                        program,
                        sourceNodeId,
                        targetNodeId
                    )
                    )
                {
                    return {
                        sourceNodeId,
                        targetNodeId
                    };
                }
            }
        }

        // --------------------------------------------------------
        // Dense-network path.
        //
        // Rejection sampling becomes inefficient when few absent
        // relationships remain. Instead, select a uniformly random
        // rank among absent relationships and locate it by scanning
        // possible ordered node pairs.
        //
        // This avoids constructing an explicit N^2 candidate array.
        // --------------------------------------------------------

        std::size_t remainingRank =
            selectIndex(
                absentCount,
                random
            );

        for (const RegulatoryNode& source :
            program.nodes())
        {
            for (const RegulatoryNode& target :
                program.nodes())
            {
                if (
                    hasInteraction(
                        program,
                        source.id,
                        target.id
                    )
                    )
                {
                    continue;
                }

                if (remainingRank == 0)
                {
                    return {
                        source.id,
                        target.id
                    };
                }

                --remainingRank;
            }
        }

        throw std::logic_error(
            "Failed to locate an absent regulatory interaction."
        );
    }

    RegulatoryInteractionTarget
        selectUniformInteractionLossTarget(
            const RegulatoryProgram& program,
            Random& random
        )
    {
        return selectExistingInteractionTarget(
            program,
            random
        );
    }

    std::uint64_t selectUniformDuplicationTarget(
        const RegulatoryProgram& program,
        Random& random
    )
    {
        const std::size_t index =
            selectIndex(
                program.nodeCount(),
                random
            );

        return program.nodes()[index].id;
    }

    std::uint64_t selectUniformNodeLossTarget(
        const RegulatoryProgram& program,
        Random& random
    )
    {
        if (program.nodeCount() <= 1)
        {
            throw std::logic_error(
                "Final regulatory node cannot be selected for deletion."
            );
        }

        const std::size_t index =
            selectIndex(
                program.nodeCount(),
                random
            );

        return program.nodes()[index].id;
    }

} // namespace ae