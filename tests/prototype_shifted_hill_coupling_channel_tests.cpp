#include "alien_evolution/physics/PrototypeShiftedHillCouplingChannel.hpp"
#include "alien_evolution/genetics/RegulatoryDynamics.hpp"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace
{
    using Channel = ae::PrototypeShiftedHillCouplingChannel;
    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }
    template<class Function> void rejects(Function function)
    {
        try { function(); }
        catch (const std::invalid_argument&) { return; }
        throw std::runtime_error("Malformed channel parameter/input accepted.");
    }
    bool exact(double a, double b)
    {
        return std::bit_cast<std::uint64_t>(a) == std::bit_cast<std::uint64_t>(b);
    }
    void sameState(const ae::RegulatoryState& a, const ae::RegulatoryState& b)
    {
        require(a.size() == b.size(), "Regulatory state size changed.");
        for (std::size_t i = 0; i < a.size(); ++i)
            require(exact(a[i], b[i]), "Downstream regulatory result differs bitwise.");
    }
    // A controlled no-interaction fixture absorbs the supplied modulation into
    // basal production, allowing both values into the same unchanged dynamics API.
    ae::RegulatoryProgram withModulation(const ae::RegulatoryProgram& program, double modulation)
    {
        auto nodes = program.nodes();
        nodes[0].basalProductionRate *= modulation;
        return ae::RegulatoryProgram(std::move(nodes), program.interactions());
    }
    static_assert(std::is_same_v<decltype(std::declval<Channel&>().schema()), const ae::PhysicalCouplingProcessSchema&>);
    static_assert(std::is_same_v<decltype(std::declval<Channel&>().scientificMetadata()), const ae::ScientificModelMetadata&>);
    static_assert(std::is_same_v<decltype(std::declval<Channel&>().adaptivePhysicsContract()), const ae::AdaptivePhysicsContract&>);
}

int main()
{
    try
    {
        const Channel activation(5.0, 2.0, 2.0), repression(0.25, 2.0, 2.0), neutral(1.0, 2.0, 2.0);
        require(activation.foldChange() == 5.0 && activation.halfSaturation() == 2.0 &&
            activation.cooperativity() == 2.0, "Read-only parameter values changed.");
        require(activation.evaluate(0.0) == 1.0 && activation.evaluate(2.0) == 3.0 &&
            repression.evaluate(2.0) == 0.625 && neutral.evaluate(1e300) == 1.0,
            "Activation/repression/neutral behavior changed.");
        const auto infinity = std::numeric_limits<double>::infinity();
        const auto nan = std::numeric_limits<double>::quiet_NaN();
        for (double invalid : {0.0, -1.0, infinity, -infinity, nan})
        {
            rejects([&] { Channel bad(invalid, 1.0, 1.0); });
            rejects([&] { Channel bad(1.0, invalid, 1.0); });
            rejects([&] { Channel bad(1.0, 1.0, invalid); });
        }
        for (double invalid : {-1.0, infinity, -infinity, nan})
            rejects([&] { (void)activation.evaluate(invalid); });

        const ae::RegulatoryProgram target({{10, 0.2, 2.0, 1.0}}, {});
        std::size_t comparisons = 0;
        for (double fold : {0.05, 0.25, 0.999, 1.0, 1.001, 5.0, 1e100})
        for (double scale : {1e-200, 0.1, 1.0, 10.0, 1e200})
        for (double exponent : {0.1, 0.5, 1.0, 2.0, 8.0, 100.0})
        {
            const Channel channel(fold, scale, exponent);
            const ae::RegulatoryInputInterface legacy(target, {{100, 10, fold, scale, exponent}});
            const std::array inputs{0.0, -0.0, scale * 0.01, scale * 0.5,
                std::nextafter(scale, 0.0), scale, std::nextafter(scale, infinity),
                scale * 2.0, scale * 100.0, 1e300, std::numeric_limits<double>::max()};
            for (double input : inputs)
            {
                const auto value = channel.evaluate(input);
                require(exact(value, ae::RegulatoryDynamics::shiftedHill(input, scale, exponent, fold)),
                    "Direct stable-kernel compatibility failed.");
                require(exact(value, legacy.modulationFactor(10, {{100, input}})),
                    "Single-channel V0.3 compatibility failed.");
                require(exact(value, channel.evaluate(input)), "Repeated evaluation is not deterministic.");
                require(std::isfinite(value) && value > 0.0, "Ordinary response matrix produced invalid modulation.");
                ++comparisons;
            }
        }
        // Preserve numerical semantics even at finite extremes; no domain broadening.
        const auto smallest = std::numeric_limits<double>::denorm_min();
        const auto largest = std::numeric_limits<double>::max();
        for (double fold : {smallest, largest})
        for (double scale : {smallest, largest})
        for (double exponent : {smallest, largest})
        {
            const Channel channel(fold, scale, exponent);
            const ae::RegulatoryInputInterface legacy(target, {{100, 10, fold, scale, exponent}});
            for (double input : {0.0, smallest, 1.0, largest})
                require(exact(channel.evaluate(input), legacy.modulationFactor(10, {{100, input}})),
                    "Extreme finite parameter compatibility changed.");
        }
        require(Channel(smallest, 1.0, 1.0).evaluate(largest) == 0.0,
            "Legacy extreme repression cancellation was silently repaired.");

        const auto& metadata = activation.scientificMetadata();
        const auto& schema = activation.schema();
        const auto& contract = activation.adaptivePhysicsContract();
        metadata.validate(); schema.validate(); contract.validate();
        require(metadata.identifier == "alien_evolution.prototype.shifted_hill_pcc" && metadata.version == "m6-v1" &&
            schema.identifier == "alien_evolution.prototype.shifted_hill.channel", "Stable model/process identity changed.");
        require(&metadata == &schema.model && metadata.description->find("V0.3") != std::string::npos &&
            metadata.description->find("not biological calibration") != std::string::npos && metadata.evidence.empty(),
            "Prototype provenance or evidence boundary changed.");
        require(metadata.parameters.size() == 3 && metadata.parameters[0].key == "foldChange" &&
            metadata.parameters[1].key == "halfSaturation" &&
            metadata.parameters[1].unit == "caller/model-defined input unit" &&
            metadata.parameters[2].key == "cooperativity" && metadata.assumptions.size() == 6 &&
            metadata.validityScope.find("compatibility only") != std::string::npos &&
            metadata.uncertainties == std::vector<ae::UncertaintyKind>{ae::UncertaintyKind::NumericalReduction,
                ae::UncertaintyKind::ScientificModelForm}, "Parameter/scope/uncertainty metadata changed.");
        require(schema.ports.size() == 1 && schema.ports[0].directionality == ae::PortDirectionality::Input &&
            schema.ports[0].quantity.typeIdentifier == "prototype.shifted_hill.external_scalar" &&
            schema.observables.size() == 1 && schema.observables[0].key == "modulation_factor" &&
            schema.observables[0].unit == "dimensionless" && schema.state.empty() && schema.history.empty() &&
            schema.control.empty() && schema.ledger.empty() && !schema.regionBindingIdentifier,
            "Reduced schema acquired physical-domain/state/exchange semantics.");
        require(contract.model == ae::ScientificModelRef(metadata.identifier, metadata.version) &&
            contract.quantitiesOfInterest.size() == 1 && contract.quantitiesOfInterest[0].key == schema.observables[0].key &&
            contract.validityCriteria.size() == 1 && contract.validityCriteria[0].description.find("no internal state/history") != std::string::npos &&
            contract.validityCriteria[0].description.find("intrinsic physical noise") != std::string::npos &&
            contract.errorMeasures.size() == 1 && contract.errorMeasures[0].quantityOfInterestKey == "modulation_factor" &&
            contract.errorMeasures[0].description->find("does not imply zero scientific/model-form uncertainty") != std::string::npos &&
            contract.referenceModels == std::vector<ae::ScientificModelRef>{{"alien_evolution.prototype.v03.shifted_hill_external_input", "v0.3"}},
            "QoI/regime/compatibility reference contract changed.");

        const ae::RegulatoryProgram program({{10, 0.2, 2.0, 1.0}, {20, 0.3, 3.0, 0.5}}, {});
        const auto state = ae::RegulatoryDynamics::initialState(program);
        for (double fold : {0.25, 1.0, 5.0})
        {
            const Channel channel(fold, 2.0, 2.0);
            const ae::RegulatoryInputInterface legacy(program, {{100, 10, fold, 2.0, 2.0}});
            for (double input : {0.0, 0.1, 2.0, 100.0})
            {
                const std::vector<ae::ExternalSignalValue> signals{{100, input}};
                const auto legacyProgram = withModulation(program, legacy.modulationFactor(10, signals));
                const auto pccProgram = withModulation(program, channel.evaluate(input));
                sameState(ae::RegulatoryDynamics::derivatives(legacyProgram, state),
                    ae::RegulatoryDynamics::derivatives(pccProgram, state));
                sameState(ae::RegulatoryDynamics::derivatives(program, state, legacy, signals),
                    ae::RegulatoryDynamics::derivatives(pccProgram, state));
                for (double step : {0.001, 0.1})
                {
                    sameState(ae::RegulatoryDynamics::stepRK4(legacyProgram, state, step),
                        ae::RegulatoryDynamics::stepRK4(pccProgram, state, step));
                    sameState(ae::RegulatoryDynamics::stepRK4(program, state, step, legacy, signals),
                        ae::RegulatoryDynamics::stepRK4(pccProgram, state, step));
                }
            }
        }
        std::cout << "All prototype shifted-Hill PCC tests passed (" << comparisons
            << " ordinary scalar comparisons plus extremes and integration).\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
