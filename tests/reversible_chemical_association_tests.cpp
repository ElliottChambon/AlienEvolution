#include "alien_evolution/physics/ReversibleChemicalAssociation.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace
{
    using Model = ae::ReversibleChemicalAssociation;
    using Profile = ae::ChemicalAssociationRepresentation;
    void require(bool value, const char* message)
    {
        if (!value) throw std::runtime_error(message);
    }
    void near(double a, double b, double tolerance = 2e-13)
    {
        require(std::isfinite(a) && std::isfinite(b) &&
            std::abs(a - b) <= tolerance * std::max({1.0, std::abs(a), std::abs(b)}), "Numerical comparison failed");
    }
    template<class Exception = std::invalid_argument, class Function> void rejects(Function function)
    {
        try { function(); } catch (const Exception&) { return; }
        throw std::runtime_error("Expected explicit rejection");
    }
    Model model(double ka = 1.0, double kd = 0.2)
    {
        return Model({1.0, 1.0 / (4.0 * std::numbers::pi), ka, kd});
    }
    void parametersAndRates()
    {
        const double inf = std::numeric_limits<double>::infinity();
        const double nan = std::numeric_limits<double>::quiet_NaN();
        for (double bad : {0.0, -1.0, inf, -inf, nan})
        {
            rejects([&] { ae::ChemicalAssociationParameters p(bad, 1, 1, 1); });
            rejects([&] { ae::ChemicalAssociationParameters p(1, bad, 1, 1); });
            rejects([&] { ae::ChemicalAssociationParameters p(1, 1, bad, 1); });
        }
        for (double bad : {-1.0, inf, -inf, nan})
            rejects([&] { ae::ChemicalAssociationParameters p(1, 1, 1, bad); });
        for (double chi : {0.01, 0.1, 1.0, 10.0, 100.0})
        {
            const auto m = model(chi, 0.2 * chi);
            const auto& r = m.rates();
            near(r.diffusionLimitedAssociationRate, 1.0);
            near(r.intrinsicToDiffusionRatio, chi);
            near(r.effectiveAssociationRate, chi / (1.0 + chi));
            near(r.rebindingProbability, chi / (chi + 1.0));
            near(r.effectiveDissociationRate, 0.2 * chi / (chi + 1.0));
            near(r.dissociationConcentration, 0.2);
            near(r.effectiveDissociationRate / r.effectiveAssociationRate, r.dissociationConcentration);
            require(r.effectiveAssociationRate > 0 && r.effectiveAssociationRate <= r.diffusionLimitedAssociationRate,
                "Invalid effective rate");
        }
        near(model().rates().effectiveAssociationRate, 0.5);
        near(model(1e-8).rates().effectiveAssociationRate / 1e-8, 1.0, 2e-8);
        near(model(1e8).rates().effectiveAssociationRate, 1.0, 2e-8);
        // An intermediate product would overflow in naive k_D*k_a/(k_D+k_a).
        const Model large({1e200, 1e-200, 1e308, 1.0});
        near(large.rates().diffusionLimitedAssociationRate, 4 * std::numbers::pi);
        require(std::isfinite(large.rates().effectiveAssociationRate), "Avoidable rate overflow");
        rejects<std::overflow_error>([] { Model m({1e308, 1e308, 1, 1}); });
        rejects<std::overflow_error>([] { Model m({1e-300, 1e-300, 1, 1}); });
    }
    void deterministic()
    {
        const auto m = model();
        const auto K = m.rates().dissociationConcentration;
        near(m.equilibriumOccupancy(0), 0);
        near(m.equilibriumOccupancy(K), 0.5);
        near(m.equilibriumOccupancy(1e8 * K), 1, 2e-8);
        require(m.equilibriumOccupancy(std::numeric_limits<double>::max()) <= 1, "Saturation overflow");
        for (double c : {0.0, K, 2 * K, 100 * K})
        for (double initial : {0.0, 0.1, 0.5, 1.0})
        {
            const double equilibrium = c / (K + c);
            const double hazard = m.rates().effectiveAssociationRate * c + m.rates().effectiveDissociationRate;
            near(m.relaxationTime(c), 1 / hazard);
            near(m.occupancyDerivative(initial, c), m.rates().effectiveAssociationRate * c * (1 - initial)
                - m.rates().effectiveDissociationRate * initial);
            for (double time : {0.0, 1e-8, 0.1, 1.0, 10.0, 1000.0})
            {
                const auto response = m.occupancyAfter(initial, c, time);
                near(response, equilibrium + (initial - equilibrium) * std::exp(-hazard * time));
                require(response >= 0 && response <= 1, "Probability left [0,1]");
                near(m.occupancyAfter(m.occupancyAfter(initial, c, time), c, time),
                    m.occupancyAfter(initial, c, 2 * time));
            }
            near(m.occupancyDerivative(equilibrium, c), 0);
        }
        // Exact successive constant-c segments cover a reservoir step, no ODE discretization.
        const double before = m.occupancyAfter(0, K, 1);
        const double after = m.occupancyAfter(before, 2 * K, 2);
        near(after, 2.0 / 3.0 + (before - 2.0 / 3.0) * std::exp(-0.3 * 2));
        const auto faster = model(2, 0.4);
        near(faster.equilibriumOccupancy(K), m.equilibriumOccupancy(K));
        require(faster.relaxationTime(K) < m.relaxationTime(K), "Affinity incorrectly fixes kinetics");
        near(faster.relaxationTime(K) / m.relaxationTime(K), 0.75);

        const auto absorbing = model(1, 0);
        rejects<std::domain_error>([&] { (void)absorbing.equilibriumOccupancy(0); });
        near(absorbing.equilibriumOccupancy(1), 1);
        require(std::isinf(absorbing.relaxationTime(0)), "Frozen state has a finite relaxation time");
        near(absorbing.occupancyAfter(0.3, 0, 100), 0.3);
        near(absorbing.occupancyDerivative(0.3, 0), 0);
        for (double bad : {-1.0, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()})
        {
            rejects([&] { (void)m.equilibriumOccupancy(bad); });
            rejects([&] { (void)m.relaxationTime(bad); });
            rejects([&] { (void)m.occupancyDerivative(0.5, bad); });
            rejects([&] { (void)m.occupancyAfter(0.5, bad, 1); });
            ae::Random rng(1);
            rejects([&] { (void)m.sampleTrajectory(false, bad, 1, rng); });
            rejects([&] { (void)m.occupancyAfter(0.5, 1, bad); });
            rejects([&] { (void)m.sampleTrajectory(false, 1, bad, rng); });
        }
        for (double bad : {-0.1, 1.1, std::numeric_limits<double>::quiet_NaN()})
        {
            rejects([&] { (void)m.occupancyDerivative(bad, 1); });
            rejects([&] { (void)m.occupancyAfter(bad, 1, 1); });
        }
    }
    void stochastic()
    {
        const auto m = model();
        ae::Random first(3501), second(3501);
        const auto a = m.sampleTrajectory(false, 0.2, 100, first);
        const auto b = m.sampleTrajectory(false, 0.2, 100, second);
        require(!a.events.empty() && a.events.size() == b.events.size(), "Trajectory mismatch");
        bool bound = false;
        double time = 0;
        for (std::size_t i = 0; i < a.events.size(); ++i)
        {
            require(a.events[i].time > time && a.events[i].time <= a.duration, "Invalid event time");
            require(a.events[i].bound != bound, "CTMC transition did not change binary state");
            require(a.events[i].time == b.events[i].time && a.events[i].bound == b.events[i].bound,
                "Seeded trajectory differs");
            time = a.events[i].time; bound = a.events[i].bound;
        }
        require(a.finallyBound == bound && b.finallyBound == bound && first.raw() == second.raw(), "RNG/state mismatch");
        ae::Random zero(8), control(8);
        require(m.sampleTrajectory(true, 1, 0, zero).events.empty() && zero.raw() == control.raw(), "Zero horizon draws RNG");
        ae::Random frozen(9), frozenControl(9);
        const auto absorbing = model(1, 0);
        require(absorbing.sampleTrajectory(true, 1, 100, frozen).finallyBound &&
            absorbing.sampleTrajectory(false, 0, 100, frozen).events.empty() && frozen.raw() == frozenControl.raw(),
            "Absorbing state draws RNG");
        require(absorbing.sampleTrajectory(false, 1, 100, frozen).events.size() == 1, "Irreversible binding not absorbing");
        rejects<std::runtime_error>([&] { (void)m.sampleTrajectory(false, 1, 1000, frozen, 0); });

        // Predeclared fast fixed-seed ensemble check: N=12000, 16 relaxation times,
        // absolute mean/variance tolerances .025, conservative versus binomial SE.
        constexpr int samples = 12000;
        constexpr double tolerance = 0.025;
        ae::Random ensemble(3502);
        const double c = 0.3, duration = 16 * m.relaxationTime(c);
        int boundCount = 0;
        for (int i = 0; i < samples; ++i)
            boundCount += m.sampleTrajectory(false, c, duration, ensemble).finallyBound ? 1 : 0;
        const double mean = static_cast<double>(boundCount) / samples;
        const double expected = m.equilibriumOccupancy(c);
        require(std::abs(mean - expected) < tolerance, "Stationary CTMC mean outside predeclared tolerance");
        require(std::abs(mean * (1 - mean) - expected * (1 - expected)) < tolerance,
            "Stationary Bernoulli variance outside predeclared tolerance");
    }
    void integration()
    {
        for (auto profile : {Profile::AnalyticalRelations, Profile::EquilibriumReduction,
            Profile::DeterministicWellMixed, Profile::StochasticWellMixed})
        {
            const auto& s = Model::schema(profile);
            const auto& m = Model::scientificMetadata(profile);
            const auto& c = Model::adaptivePhysicsContract(profile);
            s.validate(); m.validate(); c.validate();
            require(&m == &s.model && m.version == "chem1a-v1" && m.parameters.size() == 4, "M1 identity/parameters lost");
            require(m.evidence.size() == 1 && m.evidence[0].status == ae::EvidenceStatus::EstablishedPhysicalInteraction &&
                m.evidence[0].references.size() == 3 && m.evidence[0].limitation->find("not evidence of biological") != std::string::npos,
                "Evidence distinction lost");
            require(s.ports.size() == 1 && s.ports[0].directionality == ae::PortDirectionality::Input &&
                s.ports[0].quantity.unit == "1/m^3" && s.regionBindingIdentifier && s.history.empty() && s.control.empty(),
                "Schema became runtime chemistry infrastructure");
            require(c.model == ae::ScientificModelRef(m.identifier, m.version) &&
                c.quantitiesOfInterest.size() == s.observables.size() && c.errorMeasures.size() == s.observables.size() &&
                c.referenceModels.back().version() == "review-pending" &&
                c.description->find("No scalar fidelity rank") != std::string::npos, "ACP boundary lost");
        }
        require(Model::schema(Profile::AnalyticalRelations).state.empty() &&
            Model::schema(Profile::EquilibriumReduction).state.empty(), "Algebraic representation acquired history");
        require(Model::schema(Profile::DeterministicWellMixed).state[0].key == "bound_probability" &&
            Model::schema(Profile::StochasticWellMixed).state[0].key == "binary_bound_state", "Distinct state profiles collapsed");
        const auto registry = Model::spatialReferenceChallenges();
        require(registry.cases().size() == 8, "Missing spatial adversarial cases");
        for (const auto& challenge : registry.cases())
        {
            challenge.validate(Model::adaptivePhysicsContract(Profile::StochasticWellMixed));
            require(challenge.description.find("UNRESOLVED") != std::string::npos &&
                challenge.contextReference.find("spatial_solver=unselected") != std::string::npos &&
                registry.find(challenge.identifier), "Spatial challenge silently certified");
        }
        rejects([] { (void)Model::schema(static_cast<Profile>(99)); });
    }
}
int main()
{
    try
    {
        parametersAndRates(); deterministic(); stochastic(); integration();
        std::cout << "CHEM-1A analytical, deterministic, CTMC and metadata tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
