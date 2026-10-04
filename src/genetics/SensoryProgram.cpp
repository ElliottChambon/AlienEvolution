#include "alien_evolution/genetics/SensoryProgram.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace ae
{
    SensoryProgram::SensoryProgram(std::vector<RegulatoryInputChannel> channels)
        : channels_(std::move(channels))
    {
        for (const RegulatoryInputChannel& channel : channels_)
        {
            if (!std::isfinite(channel.foldChange) || channel.foldChange <= 0.0
                || !std::isfinite(channel.halfSaturation) || channel.halfSaturation <= 0.0
                || !std::isfinite(channel.cooperativity) || channel.cooperativity <= 0.0)
            {
                throw std::invalid_argument("Sensory channel parameters must be finite and positive.");
            }
        }
        // As in RegulatoryInputInterface, channels may share signal/target IDs.
        // Uniqueness applies to runtime signal values, not inherited channels.
    }

    const std::vector<RegulatoryInputChannel>& SensoryProgram::channels() const
    {
        return channels_;
    }

    std::size_t SensoryProgram::channelCount() const
    {
        return channels_.size();
    }

    RegulatoryInputInterface SensoryProgram::makeRegulatoryInputInterface(
        const RegulatoryProgram& program
    ) const
    {
        return RegulatoryInputInterface(program, channels_);
    }
} // namespace ae
