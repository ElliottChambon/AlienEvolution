#include "alien_evolution/genetics/RegulatoryInput.hpp"

#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "alien_evolution/genetics/RegulatoryDynamics.hpp"

namespace ae
{
    RegulatoryInputInterface::RegulatoryInputInterface(
        const RegulatoryProgram& program,
        std::vector<RegulatoryInputChannel> channels
    )
        : channels_(std::move(channels))
    {
        validate(program);
    }

    const std::vector<RegulatoryInputChannel>&
        RegulatoryInputInterface::channels() const
    {
        return channels_;
    }

    std::size_t RegulatoryInputInterface::channelCount() const
    {
        return channels_.size();
    }

    void RegulatoryInputInterface::validate(const RegulatoryProgram& program) const
    {
        for (const RegulatoryInputChannel& channel : channels_)
        {
            if (!program.containsNode(channel.targetNodeId))
            {
                throw std::invalid_argument("Regulatory input target node does not exist.");
            }
            if (!std::isfinite(channel.foldChange) || channel.foldChange <= 0.0
                || !std::isfinite(channel.halfSaturation) || channel.halfSaturation <= 0.0
                || !std::isfinite(channel.cooperativity) || channel.cooperativity <= 0.0)
            {
                throw std::invalid_argument("Regulatory input parameters must be finite and positive.");
            }
        }
    }

    double RegulatoryInputInterface::modulationFactor(
        const std::uint64_t targetNodeId,
        const std::vector<ExternalSignalValue>& signals
    ) const
    {
        std::unordered_map<std::uint64_t, double> values;
        for (const ExternalSignalValue& signal : signals)
        {
            if (!std::isfinite(signal.value) || signal.value < 0.0)
            {
                throw std::invalid_argument("External signal values must be finite and nonnegative.");
            }
            if (!values.emplace(signal.signalId, signal.value).second)
            {
                throw std::invalid_argument("External signal IDs must be unique.");
            }
        }

        double factor = 1.0;
        for (const RegulatoryInputChannel& channel : channels_)
        {
            if (channel.targetNodeId != targetNodeId)
            {
                continue;
            }
            const auto signal = values.find(channel.signalId);
            if (signal == values.end())
            {
                throw std::invalid_argument("Required external signal is missing.");
            }
            factor *= RegulatoryDynamics::shiftedHill(
                signal->second, channel.halfSaturation,
                channel.cooperativity, channel.foldChange
            );
            if (!std::isfinite(factor))
            {
                throw std::overflow_error("Regulatory input modulation became non-finite.");
            }
        }
        return factor;
    }
} // namespace ae
