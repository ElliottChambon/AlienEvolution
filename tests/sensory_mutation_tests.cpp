#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "alien_evolution/genetics/SensoryMutation.hpp"

namespace
{
    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    template <typename Exception, typename Function>
    void requireThrows(Function function)
    {
        try { function(); }
        catch (const Exception&) { return; }
        throw std::runtime_error("Expected sensory mutation error was not raised.");
    }

    bool sameChannel(const ae::RegulatoryInputChannel& a, const ae::RegulatoryInputChannel& b)
    {
        return a.signalId == b.signalId && a.targetNodeId == b.targetNodeId
            && a.foldChange == b.foldChange && a.halfSaturation == b.halfSaturation
            && a.cooperativity == b.cooperativity;
    }

    void requireSame(const ae::SensoryProgram& a, const ae::SensoryProgram& b)
    {
        require(a.channelCount() == b.channelCount(), "Channel count changed.");
        for (std::size_t i = 0; i < a.channelCount(); ++i)
            require(sameChannel(a.channels()[i], b.channels()[i]), "Channel data differs.");
    }
}

int main()
{
    try
    {
        const ae::SensoryProgram parent({{100, 10, 2.0, 3.0, 4.0}, {200, 20, 0.5, 1.0, 2.0}});
        require(ae::computeSensoryMutationHazard(parent, {}) == 0.0, "Default hazard nonzero.");
        require(ae::computeSensoryMutationHazard(ae::SensoryProgram{}, {2.0}) == 0.0,
            "Empty sensory program has hazard.");
        require(ae::computeSensoryMutationHazard(parent, {2.5}) == 5.0, "Incorrect per-channel hazard.");
        for (double invalid : {-1.0, std::numeric_limits<double>::infinity(),
            std::numeric_limits<double>::quiet_NaN()})
        {
            requireThrows<std::invalid_argument>([&] { (void)ae::computeSensoryMutationHazard(parent, {invalid}); });
            requireThrows<std::invalid_argument>([&] {
                (void)ae::computeSensoryMutationHazard(ae::SensoryProgram{}, {invalid});
            });
            for (int parameter = 0; parameter < 3; ++parameter)
            {
                ae::SensoryMutationEffectModel model{};
                if (parameter == 0) model.foldChangeLogStdDev = invalid;
                if (parameter == 1) model.halfSaturationLogStdDev = invalid;
                if (parameter == 2) model.cooperativityLogStdDev = invalid;
                ae::Random random(123);
                requireThrows<std::invalid_argument>([&] { (void)ae::sampleSensoryChannelParameterChange(model, random); });
                ae::SensoryMutationGeneratorConfig config{};
                config.quantitativeEffects = model;
                requireThrows<std::invalid_argument>([&] { (void)ae::generateSensoryOffspring(parent, config, random); });
            }
        }
        requireThrows<std::overflow_error>([&] {
            (void)ae::computeSensoryMutationHazard(parent, {std::numeric_limits<double>::max()});
        });
        ae::Random neutralRandom(123), untouchedRandom(123);
        const auto neutral = ae::sampleSensoryChannelParameterChange({}, neutralRandom);
        require(neutral.foldChangeMultiplier == 1.0 && neutral.halfSaturationMultiplier == 1.0
            && neutral.cooperativityMultiplier == 1.0, "Zero effect scales changed parameters.");
        require(neutralRandom.raw() == untouchedRandom.raw(), "Zero effect scales consumed random draws.");
        ae::Random sampleRandom(321), sampleOracle(321);
        const auto sampled = ae::sampleSensoryChannelParameterChange({0.1, 0.2, 0.3}, sampleRandom);
        require(sampled.foldChangeMultiplier == std::exp(sampleOracle.normal(0.0, 0.1))
            && sampled.halfSaturationMultiplier == std::exp(sampleOracle.normal(0.0, 0.2))
            && sampled.cooperativityMultiplier == std::exp(sampleOracle.normal(0.0, 0.3)),
            "Sensory effects do not follow the regulatory log-multiplier convention.");
        for (int i = 0; i < 100; ++i)
        {
            const auto change = ae::sampleSensoryChannelParameterChange({0.1, 0.2, 0.3}, sampleRandom);
            for (double value : {change.foldChangeMultiplier, change.halfSaturationMultiplier, change.cooperativityMultiplier})
                require(std::isfinite(value) && value > 0.0, "Ordinary effect sample invalid.");
        }

        const auto changed = ae::applySensoryMutationEvents(parent, {{0, {2.0, 0.5, 3.0}}, {0, {0.5, 2.0, 1.0}}});
        require(sameChannel(changed.channels()[0], {100, 10, 2.0, 3.0, 12.0})
            && sameChannel(changed.channels()[1], parent.channels()[1]), "Event composition/target isolation failed.");
        require(sameChannel(parent.channels()[0], {100, 10, 2.0, 3.0, 4.0}), "Event application modified parent.");
        requireSame(parent, ae::applySensoryMutationEvents(parent, {}));
        requireThrows<std::out_of_range>([&] { (void)ae::applySensoryMutationEvents(parent, {{2, {}}}); });
        for (double invalid : {0.0, -1.0, std::numeric_limits<double>::infinity(),
            std::numeric_limits<double>::quiet_NaN()})
        {
            for (int parameter = 0; parameter < 3; ++parameter)
            {
                ae::SensoryChannelParameterChange change{};
                if (parameter == 0) change.foldChangeMultiplier = invalid;
                if (parameter == 1) change.halfSaturationMultiplier = invalid;
                if (parameter == 2) change.cooperativityMultiplier = invalid;
                requireThrows<std::invalid_argument>([&] { (void)ae::applySensoryMutationEvents(parent, {{0, change}}); });
            }
        }
        for (int parameter = 0; parameter < 3; ++parameter)
        {
            ae::SensoryChannelParameterChange change{};
            if (parameter == 0) change.foldChangeMultiplier = std::numeric_limits<double>::max();
            if (parameter == 1) change.halfSaturationMultiplier = std::numeric_limits<double>::max();
            if (parameter == 2) change.cooperativityMultiplier = std::numeric_limits<double>::max();
            requireThrows<std::overflow_error>([&] { (void)ae::applySensoryMutationEvents(parent, {{0, change}}); });
        }
        const ae::SensoryProgram tiny({{100, 999, std::numeric_limits<double>::denorm_min(), 1.0, 1.0}});
        requireThrows<std::overflow_error>([&] { (void)ae::applySensoryMutationEvents(tiny, {{0, {0.1, 1.0, 1.0}}}); });

        ae::SensoryMutationGeneratorConfig config{};
        for (int empty = 0; empty < 2; ++empty)
        {
            config.rates.channelParameterPerChannel = empty ? 10.0 : 0.0;
            config.quantitativeEffects = {0.1, 0.2, 0.3};
            const auto& source = empty ? ae::SensoryProgram{} : parent;
            ae::Random random(111), oracle(111);
            const auto result = ae::generateSensoryOffspring(source, config, random);
            requireSame(result.offspringProgram, source);
            require(result.events.empty(), "Zero hazard produced events.");
            require(random.raw() == oracle.raw(), "Zero hazard consumed RNG draws.");
        }
        config.rates.channelParameterPerChannel = 10.0;
        ae::Random random(1234), repeatRandom(1234);
        const auto result = ae::generateSensoryOffspring(parent, config, random);
        const auto repeat = ae::generateSensoryOffspring(parent, config, repeatRandom);
        requireSame(result.offspringProgram, repeat.offspringProgram);
        require(!result.events.empty() && result.events.size() == repeat.events.size(), "Deterministic event count failed.");
        std::vector<ae::SensoryChannelParameterMutationEvent> replay;
        double previous = 0.0;
        for (std::size_t i = 0; i < result.events.size(); ++i)
        {
            const auto& a = result.events[i];
            const auto& b = repeat.events[i];
            require(a.replicationProgress > previous && a.replicationProgress <= 1.0, "Invalid event timing.");
            previous = a.replicationProgress;
            require(a.replicationProgress == b.replicationProgress && a.event.channelIndex == b.event.channelIndex
                && a.event.change.foldChangeMultiplier == b.event.change.foldChangeMultiplier
                && a.event.change.halfSaturationMultiplier == b.event.change.halfSaturationMultiplier
                && a.event.change.cooperativityMultiplier == b.event.change.cooperativityMultiplier,
                "Sensory event sequence is not deterministic.");
            replay.push_back(a.event);
        }
        requireSame(result.offspringProgram, ae::applySensoryMutationEvents(parent, replay));
        for (std::size_t i = 0; i < parent.channelCount(); ++i)
        {
            const auto& child = result.offspringProgram.channels()[i];
            require(child.signalId == parent.channels()[i].signalId && child.targetNodeId == parent.channels()[i].targetNodeId,
                "Mutation changed fixed identities/order.");
            require(std::isfinite(child.foldChange) && child.foldChange > 0.0
                && std::isfinite(child.halfSaturation) && child.halfSaturation > 0.0
                && std::isfinite(child.cooperativity) && child.cooperativity > 0.0, "Invalid offspring parameter.");
        }
        config.safetyEventLimit = 0;
        requireThrows<std::invalid_argument>([&] { (void)ae::generateSensoryOffspring(parent, config, random); });
        config.safetyEventLimit = 1;
        config.rates.channelParameterPerChannel = 1000.0;
        requireThrows<std::runtime_error>([&] { (void)ae::generateSensoryOffspring(parent, config, random); });
        std::cout << "All sensory mutation tests passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
