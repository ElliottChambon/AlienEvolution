#include "alien_evolution/genetics/SensoryMutation.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace ae
{
    namespace
    {
        void validateNonnegative(double value)
        {
            if (!std::isfinite(value) || value < 0.0)
                throw std::invalid_argument("Sensory mutation rates/scales must be finite and nonnegative.");
        }

        void validateEffects(const SensoryMutationEffectModel& model)
        {
            validateNonnegative(model.foldChangeLogStdDev);
            validateNonnegative(model.halfSaturationLogStdDev);
            validateNonnegative(model.cooperativityLogStdDev);
        }

        double sampleMultiplier(double scale, Random& random)
        {
            if (scale == 0.0) return 1.0;
            const double multiplier = std::exp(random.normal(0.0, scale));
            if (!std::isfinite(multiplier) || multiplier <= 0.0)
                throw std::overflow_error("Sensory mutation effect produced invalid multiplier.");
            return multiplier;
        }

        double applyMultiplier(double parameter, double multiplier)
        {
            if (!std::isfinite(multiplier) || multiplier <= 0.0)
                throw std::invalid_argument("Sensory mutation multipliers must be finite and positive.");
            const double result = parameter * multiplier;
            if (!std::isfinite(result) || result <= 0.0)
                throw std::overflow_error("Sensory mutation produced invalid parameter (overflow/underflow).");
            return result;
        }

        void applyEvent(std::vector<RegulatoryInputChannel>& channels,
            const SensoryChannelParameterMutationEvent& event)
        {
            if (event.channelIndex >= channels.size())
                throw std::out_of_range("Sensory mutation channel index is out of range.");
            auto& channel = channels[event.channelIndex];
            channel.foldChange = applyMultiplier(channel.foldChange, event.change.foldChangeMultiplier);
            channel.halfSaturation = applyMultiplier(channel.halfSaturation, event.change.halfSaturationMultiplier);
            channel.cooperativity = applyMultiplier(channel.cooperativity, event.change.cooperativityMultiplier);
        }
    }

    SensoryProgram applySensoryMutationEvents(const SensoryProgram& parent,
        const std::vector<SensoryChannelParameterMutationEvent>& events)
    {
        auto channels = parent.channels();
        for (const auto& event : events) applyEvent(channels, event);
        return SensoryProgram(std::move(channels));
    }

    double computeSensoryMutationHazard(const SensoryProgram& program,
        const SensoryMutationRateModel& rates)
    {
        validateNonnegative(rates.channelParameterPerChannel);
        const double hazard = rates.channelParameterPerChannel * static_cast<double>(program.channelCount());
        if (!std::isfinite(hazard)) throw std::overflow_error("Sensory mutation hazard became non-finite.");
        return hazard;
    }

    SensoryChannelParameterChange sampleSensoryChannelParameterChange(
        const SensoryMutationEffectModel& model, Random& random)
    {
        validateEffects(model);
        return {sampleMultiplier(model.foldChangeLogStdDev, random),
            sampleMultiplier(model.halfSaturationLogStdDev, random),
            sampleMultiplier(model.cooperativityLogStdDev, random)};
    }

    SensoryMutationGenerationResult generateSensoryOffspring(const SensoryProgram& parent,
        const SensoryMutationGeneratorConfig& config, Random& random)
    {
        if (config.safetyEventLimit == 0)
            throw std::invalid_argument("Sensory mutation safety event limit must be positive.");
        validateEffects(config.quantitativeEffects);
        const double hazard = computeSensoryMutationHazard(parent, config.rates);
        auto channels = parent.channels();
        std::vector<TimedSensoryMutationEvent> events;
        double progress = 0.0;
        while (hazard > 0.0 && progress < 1.0)
        {
            const double draw = std::clamp(random.uniform01(),
                std::numeric_limits<double>::min(), std::nextafter(1.0, 0.0));
            const double waiting = -std::log(draw) / hazard;
            if (waiting > 1.0 - progress) break;
            progress += waiting;
            if (events.size() >= config.safetyEventLimit)
                throw std::runtime_error("Sensory mutation event safety limit exceeded.");
            const auto index = std::min(static_cast<std::size_t>(random.uniform01()
                * static_cast<double>(channels.size())), channels.size() - 1);
            const SensoryChannelParameterMutationEvent event{
                index, sampleSensoryChannelParameterChange(config.quantitativeEffects, random)};
            applyEvent(channels, event);
            events.push_back({progress, event});
        }
        return {SensoryProgram(std::move(channels)), std::move(events)};
    }
} // namespace ae
