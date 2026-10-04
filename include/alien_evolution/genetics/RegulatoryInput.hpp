#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "alien_evolution/genetics/RegulatoryProgram.hpp"

namespace ae
{
    // Exogenous, finite, nonnegative signal magnitude in caller-defined units.
    struct ExternalSignalValue
    {
        std::uint64_t signalId = 0;
        double value = 0.0;
    };

    struct RegulatoryInputChannel
    {
        std::uint64_t signalId = 0;
        std::uint64_t targetNodeId = 0;
        double foldChange = 1.0;
        double halfSaturation = 1.0;
        double cooperativity = 1.0;
    };

    // Phenomenological external-signal/transduction abstraction; does not
    // prescribe Earth-like receptors or participate in development yet.
    class RegulatoryInputInterface
    {
    public:
        RegulatoryInputInterface(
            const RegulatoryProgram& program,
            std::vector<RegulatoryInputChannel> channels
        );

        [[nodiscard]] const std::vector<RegulatoryInputChannel>& channels() const;
        [[nodiscard]] std::size_t channelCount() const;

        // Revalidate against a changed program; no program reference is retained.
        void validate(const RegulatoryProgram& program) const;

        // Requires signals used by this target only. Extra signals are allowed,
        // but all supplied values must be finite, nonnegative and uniquely named.
        // A target without channels returns 1.0. Channels may share a signal ID.
        [[nodiscard]] double modulationFactor(
            std::uint64_t targetNodeId,
            const std::vector<ExternalSignalValue>& signals
        ) const;

    private:
        std::vector<RegulatoryInputChannel> channels_;
    };
} // namespace ae
