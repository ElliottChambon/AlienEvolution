#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "alien_evolution/genetics/RegulatoryInput.hpp"

namespace
{
    void require(const bool condition, const char* message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    void requireNear(const double actual, const double expected, const char* message)
    {
        require(std::isfinite(actual) && std::abs(actual - expected) < 1e-12, message);
    }

    template <typename Exception = std::invalid_argument, typename Function>
    void requireThrows(Function function, const char* message)
    {
        try
        {
            function();
        }
        catch (const Exception&)
        {
            return;
        }
        throw std::runtime_error(message);
    }
}

int main()
{
    try
    {
        const ae::RegulatoryProgram program({{10, 0.0, 1.0, 1.0}, {20, 0.0, 1.0, 1.0}}, {});
        const ae::RegulatoryInputInterface input(program, {{100, 10, 5.0, 2.0, 2.0}});
        require(input.channelCount() == 1, "Incorrect channel count.");
        const auto& channel = input.channels().at(0);
        require(channel.signalId == 100 && channel.targetNodeId == 10
            && channel.foldChange == 5.0 && channel.halfSaturation == 2.0
            && channel.cooperativity == 2.0, "Channel read access changed parameters.");
        input.validate(program);
        requireThrows([&] {
            const ae::RegulatoryInputInterface invalid(program, {{100, 999, 2.0, 1.0, 1.0}});
        }, "Missing target accepted.");
        requireThrows([&] {
            input.validate(ae::RegulatoryProgram({{20, 0.0, 1.0, 1.0}}, {}));
        }, "Revalidation accepted removed target.");

        for (const double invalid : {0.0, -1.0, std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()})
        {
            for (int parameter = 0; parameter < 3; ++parameter)
            {
                ae::RegulatoryInputChannel bad{100, 10, 5.0, 2.0, 2.0};
                if (parameter == 0) bad.foldChange = invalid;
                if (parameter == 1) bad.halfSaturation = invalid;
                if (parameter == 2) bad.cooperativity = invalid;
                requireThrows([&] {
                    const ae::RegulatoryInputInterface rejected(program, {bad});
                }, "Invalid channel parameter accepted.");
            }
        }

        const ae::RegulatoryInputInterface empty(program, {});
        require(empty.channelCount() == 0, "Empty interface has channels.");
        requireNear(empty.modulationFactor(10, {}), 1.0, "Empty interface is not neutral.");
        requireNear(input.modulationFactor(20, {}), 1.0, "Untargeted node is not neutral.");
        requireNear(input.modulationFactor(10, {{100, 0.0}}), 1.0, "Zero signal is engaged.");
        requireNear(input.modulationFactor(10, {{100, 2.0}}), 3.0, "Incorrect half-saturation response.");
        // With x/K = 2 and n = 2, engagement is 4/5.
        requireNear(input.modulationFactor(10, {{100, 4.0}}), 4.2, "Incorrect cooperative response.");
        double previous = 1.0;
        for (const double signal : {0.0, 0.5, 2.0, 20.0, 1e100, std::numeric_limits<double>::max()})
        {
            const double factor = input.modulationFactor(10, {{100, signal}});
            require(factor >= previous && factor <= 5.0, "Activation is not monotonic and bounded.");
            previous = factor;
        }
        requireNear(previous, 5.0, "Strong activation did not saturate.");

        const ae::RegulatoryInputInterface repression(program, {{100, 10, 0.25, 2.0, 2.0}});
        requireNear(repression.modulationFactor(10, {{100, 0.0}}), 1.0, "Zero repression is engaged.");
        requireNear(repression.modulationFactor(10, {{100, 2.0}}), 0.625, "Incorrect repression midpoint.");
        previous = 1.0;
        for (const double signal : {0.0, 0.5, 2.0, 20.0, 1e100})
        {
            const double factor = repression.modulationFactor(10, {{100, signal}});
            require(factor <= previous && factor >= 0.25, "Repression is not monotonic and bounded.");
            previous = factor;
        }
        requireNear(previous, 0.25, "Strong repression did not saturate.");

        const ae::RegulatoryInputInterface combined(program, {
            {100, 10, 5.0, 2.0, 2.0}, {200, 10, 0.25, 4.0, 1.0},
            {100, 20, 3.0, 2.0, 1.0}
        });
        requireNear(combined.modulationFactor(10, {{100, 2.0}, {200, 4.0}}),
            1.875, "Independent channels did not multiply.");
        requireNear(combined.modulationFactor(10, {{200, 4.0}, {100, 2.0}}),
            1.875, "Signal ordering changed modulation.");
        requireNear(combined.modulationFactor(20, {{100, 2.0}}), 2.0,
            "Shared signal or target isolation failed.");
        const ae::RegulatoryInputInterface shared(program, {
            {100, 10, 5.0, 2.0, 2.0}, {100, 10, 3.0, 2.0, 1.0}
        });
        requireNear(shared.modulationFactor(10, {{100, 2.0}}), 6.0,
            "Channels sharing a signal did not combine.");
        const ae::RegulatoryInputInterface neutral(program, {{100, 10, 1.0, 2.0, 2.0}});
        requireNear(neutral.modulationFactor(10, {{100, 100.0}}), 1.0, "Neutral channel changed factor.");

        requireThrows([&] { (void)input.modulationFactor(10, {}); }, "Missing signal accepted.");
        requireThrows([&] { (void)combined.modulationFactor(10, {{100, 2.0}}); },
            "Missing second signal accepted.");
        requireThrows([&] { (void)input.modulationFactor(10, {{100, 2.0}, {100, 3.0}}); },
            "Duplicate signal IDs accepted.");
        requireThrows([&] { (void)empty.modulationFactor(10, {{200, 1.0}, {200, 1.0}}); },
            "Unused duplicate signal IDs accepted.");
        for (const double invalid : {-1.0, std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()})
        {
            requireThrows([&] { (void)input.modulationFactor(10, {{100, invalid}}); },
                "Invalid external signal accepted.");
        }
        requireNear(input.modulationFactor(10, {{100, 2.0}, {999, 3.0}}), 3.0,
            "Extra signal changed modulation.");
        const ae::RegulatoryInputInterface overflow(program, {
            {100, 10, 1e200, 1.0, 1.0}, {200, 10, 1e200, 1.0, 1.0}
        });
        requireThrows<std::overflow_error>([&] {
            (void)overflow.modulationFactor(10, {{100, 1e100}, {200, 1e100}});
        }, "Non-finite product accepted.");

        std::cout << "All regulatory input tests passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
