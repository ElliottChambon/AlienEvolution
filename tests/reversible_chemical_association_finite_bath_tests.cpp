#include "alien_evolution/physics/ReversibleChemicalAssociationFiniteBath.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <vector>

namespace
{
    using Bath = ae::ReversibleChemicalAssociationFiniteBath;

    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    void nearRelative(double actual, double expected, double tolerance = 3.0e-9)
    {
        require(std::isfinite(actual) && std::isfinite(expected), "Nonfinite comparison");
        const auto scale = std::max({1.0e-300, std::abs(actual), std::abs(expected)});
        require(std::abs(actual - expected) <= tolerance * scale, "Relative numerical comparison failed");
    }

    template <class Exception = std::invalid_argument, class Function>
    void rejects(Function function)
    {
        try { function(); }
        catch (const Exception&) { return; }
        throw std::runtime_error("Expected exception was not thrown");
    }

    ae::ChemicalAssociationParameters parameters(double chi, double delta)
    {
        constexpr double a = 1.0;
        constexpr double D = 1.0;
        return {
            a,
            D,
            4.0 * std::numbers::pi * chi,
            delta};
    }

    ae::FiniteChemicalBathConfig config(
        double lambda = 2.0,
        double tauMin = 1.0e-4,
        double tolerance = 1.0e-12,
        std::size_t maxModes = 32768)
    {
        return {lambda, tolerance, tauMin, maxModes};
    }

    void constructionAndModes()
    {
        const Bath bath(parameters(1.0, 1.0), config());
        const auto& d = bath.diagnostics();
        nearRelative(d.chi, 1.0, 5.0e-14);
        nearRelative(d.delta, 1.0, 5.0e-14);
        nearRelative(d.lambda, 2.0, 5.0e-14);
        nearRelative(d.accessibleVolume, 28.0 * std::numbers::pi / 3.0, 5.0e-14);
        nearRelative(d.minimumResolvedPhysicalTime, 1.0e-4, 5.0e-14);
        require(d.reactiveModeCount >= 16 && d.reflectingModeCount >= 16,
            "CHEM-1C did not compile sufficient spectral modes");

        const auto reactive = bath.reactiveDimensionlessModeRoots();
        const auto reflecting = bath.reflectingDimensionlessModeRoots();
        require(reactive.size() == d.reactiveModeCount
            && reflecting.size() == d.reflectingModeCount,
            "CHEM-1C mode diagnostics mismatch");

        const double ell = d.lambda - 1.0;
        const double HOuter = 1.0 / d.lambda;
        for (std::size_t i = 0; i < std::min<std::size_t>(reactive.size(), 12); ++i)
        {
            const auto z = reactive[i];
            const auto n = static_cast<double>(i);
            const auto phase =
                z * ell
                - std::atan((1.0 + d.chi) / z)
                + std::atan(HOuter / z)
                - n * std::numbers::pi;
            const auto determinant =
                z * ((1.0 + d.chi) - HOuter) * std::cos(z * ell)
                - (z * z + (1.0 + d.chi) * HOuter) * std::sin(z * ell);
            require(std::abs(phase) < 2.0e-12 && std::abs(determinant) < 2.0e-9,
                "CHEM-1C Robin-Neumann root residual too large");
            if (i) require(reactive[i] > reactive[i - 1], "Reactive modes are not ordered");
        }

        for (std::size_t i = 0; i < std::min<std::size_t>(reflecting.size(), 12); ++i)
        {
            const auto z = reflecting[i];
            const auto n = static_cast<double>(i + 1);
            const auto phase =
                z * ell
                - std::atan(1.0 / z)
                + std::atan(HOuter / z)
                - n * std::numbers::pi;
            require(std::abs(phase) < 2.0e-12, "CHEM-1C Neumann-Neumann root residual too large");
            if (i) require(reflecting[i] > reflecting[i - 1], "Reflecting modes are not ordered");
        }

        rejects([&] { (void)Bath(parameters(1.0, 1.0), config(1.0)); });
        rejects([&] {
            auto bad = config();
            bad.spectralTolerance = 1.0;
            (void)Bath(parameters(1.0, 1.0), bad);
        });
        rejects([&] {
            auto bad = config();
            bad.minimumResolvedDimensionlessTime = 0.0;
            (void)Bath(parameters(1.0, 1.0), bad);
        });
        rejects<std::domain_error>([&] {
            auto bad = config(2.0, 1.0e-8, 1.0e-12, 16);
            (void)Bath(parameters(1.0, 1.0), bad);
        });
    }

    struct Golden
    {
        double chi;
        double lambda;
        double rho0;
        double tau;
        double rho;
        double survival;
        double firstReactionDensity;
        double shellDensity;
        double conditionalCdf;
    };

    void spectralGoldenValues()
    {
        // Independently generated from the issue #39 Robin-Neumann spectral
        // formulas with mpmath at 60 decimal digits. a=D=1.
        const std::vector<Golden> fixtures{
            {1.0, 2.0, 1.2, 0.1, 1.5,
                0.8938441707077912370,
                0.6791086183742368341,
                1.0269753873210216715,
                0.5483776716347065149},
            {10.0, 2.0, 1.0, 0.01, 1.2,
                0.4561186005786319034,
                12.22860768476116780,
                1.3003444077837114965,
                0.7493822687029371244},
            {0.1, 5.0, 3.0, 1.0, 4.0,
                0.9979657214664383657,
                0.00361401765734588051,
                0.3429219103224162419,
                0.6371233625769217697}
        };

        for (const auto& f : fixtures)
        {
            const Bath bath(parameters(f.chi, 1.0),
                config(f.lambda, 1.0e-4, 1.0e-12, 32768));
            nearRelative(bath.reactiveSurvival(f.rho0, f.tau), f.survival);
            nearRelative(
                bath.reactiveFirstReactionDensity(f.rho0, f.tau),
                f.firstReactionDensity);
            nearRelative(
                bath.reactiveRadialShellProbabilityDensity(f.rho, f.rho0, f.tau),
                f.shellDensity);
            nearRelative(
                bath.reactiveConditionalRadialCdf(f.rho, f.rho0, f.tau),
                f.conditionalCdf);
        }
    }

    void cdfAndEquilibriumChecks()
    {
        const Bath bath(parameters(1.0, 1.0), config());

        double previous = 0.0;
        for (int i = 0; i <= 20; ++i)
        {
            const auto r = 1.0 + static_cast<double>(i) / 20.0;
            const auto cdf = bath.reactiveConditionalRadialCdf(r, 1.2, 0.1);
            require(cdf + 2.0e-10 >= previous && cdf >= 0.0 && cdf <= 1.0,
                "Reactive conditional CDF is not monotone/in range");
            previous = cdf;
        }
        nearRelative(previous, 1.0, 1.0e-12);

        previous = 0.0;
        for (int i = 0; i <= 20; ++i)
        {
            const auto r = 1.0 + static_cast<double>(i) / 20.0;
            const auto cdf = bath.reflectingRadialCdf(r, 1.2, 0.1);
            require(cdf + 2.0e-10 >= previous && cdf >= 0.0 && cdf <= 1.0,
                "Reflecting CDF is not monotone/in range");
            previous = cdf;
        }
        nearRelative(previous, 1.0, 1.0e-12);

        const auto longTime = 50.0;
        const auto uniformCdf = (1.5 * 1.5 * 1.5 - 1.0) / (8.0 - 1.0);
        nearRelative(bath.reflectingRadialCdf(1.5, 1.2, longTime), uniformCdf, 2.0e-9);

        nearRelative(bath.meanFirstReactionTime(1.2), 2.7044444444444444444, 3.0e-13);
        nearRelative(bath.equilibriumBoundProbability(1), 3.0 / 10.0, 3.0e-13);
        nearRelative(bath.equilibriumBoundProbability(2), 6.0 / 13.0, 3.0e-13);
        require(bath.equilibriumBoundProbability(0) == 0.0, "Zero-ligand equilibrium must be unbound");

        const auto cTotal = 2.0 / bath.accessibleVolume();
        const ae::ReversibleChemicalAssociation reduced(parameters(1.0, 1.0));
        nearRelative(
            bath.equilibriumBoundProbability(2),
            reduced.equilibriumOccupancy(cTotal),
            3.0e-13);

        rejects<std::domain_error>([&] {
            (void)bath.reactiveSurvival(1.2, 0.5 * bath.diagnostics().minimumResolvedPhysicalTime);
        });
    }

    void stateAndTrajectory()
    {
        const Bath bath(parameters(0.2, 1.0), config());
        ae::FiniteChemicalBathState initial{{1.3, 1.7}, std::nullopt};
        bath.validateState(initial);

        rejects([&] {
            ae::FiniteChemicalBathState bad{{0.9, 1.5}, std::nullopt};
            bath.validateState(bad);
        });
        rejects([&] {
            ae::FiniteChemicalBathState bad{{1.0, 1.5}, std::size_t{1}};
            bath.validateState(bad);
        });

        ae::Random first(3901), second(3901);
        const auto a = bath.sampleTrajectory(initial, 5.0, first);
        const auto b = bath.sampleTrajectory(initial, 5.0, second);
        require(!a.events.empty() && a.events.size() == b.events.size(),
            "CHEM-1C seeded trajectory did not exercise/reproduce events");
        require(a.finalState.boundLigand == b.finalState.boundLigand
            && a.finalState.ligandRadii == b.finalState.ligandRadii
            && first.raw() == second.raw(),
            "CHEM-1C seeded trajectory/RNG state is not reproducible");

        double priorTime = 0.0;
        bool bound = false;
        for (std::size_t i = 0; i < a.events.size(); ++i)
        {
            const auto& event = a.events[i];
            require(event.time > priorTime && event.time <= a.duration,
                "CHEM-1C event time is not strictly increasing/in horizon");
            require(event.ligandIndex < initial.ligandRadii.size(), "CHEM-1C event ligand index invalid");
            if (event.kind == ae::FiniteChemicalBathEventKind::Binding)
            {
                require(!bound, "CHEM-1C double binding occurred");
                bound = true;
            }
            else
            {
                require(bound, "CHEM-1C dissociation occurred while site was free");
                bound = false;
            }
            priorTime = event.time;
        }
        bath.validateState(a.finalState);
        require(a.finalState.ligandRadii.size() == initial.ligandRadii.size(),
            "CHEM-1C conserved ligand bookkeeping changed population size");

        const Bath irreversible(parameters(0.2, 0.0), config());
        ae::FiniteChemicalBathState occupied{{1.0, 1.5}, std::size_t{0}};
        ae::Random frozen(3902);
        const auto held = irreversible.sampleTrajectory(occupied, 1.0, frozen);
        require(held.events.empty() && held.finalState.boundLigand == std::optional<std::size_t>{0},
            "Irreversible occupied target allowed competitor binding/dissociation");
        require(held.finalState.ligandRadii[0] == 1.0
            && held.finalState.ligandRadii[1] >= 1.0
            && held.finalState.ligandRadii[1] <= 2.0,
            "CHEM-1C reflecting competitor propagation violated conserved state");

        ae::Random dissociationRandom(3903);
        ae::FiniteChemicalBathState boundStart{{1.0, 1.6}, std::size_t{0}};
        const auto dissociating = bath.sampleTrajectory(boundStart, 10.0, dissociationRandom);
        require(!dissociating.events.empty()
            && dissociating.events.front().kind == ae::FiniteChemicalBathEventKind::Dissociation
            && dissociating.events.front().ligandIndex == 0,
            "CHEM-1C bound trajectory did not dissociate the same bound ligand first");
        bath.validateState(dissociating.finalState);
    }

    void metadataScope()
    {
        const auto& metadata = Bath::scientificMetadata();
        const auto& schema = Bath::schema();
        const auto& contract = Bath::adaptivePhysicsContract();
        metadata.validate();
        schema.validate();
        contract.validate();

        require(metadata.identifier == "alien_evolution.chem1.finite_bath_reference"
            && metadata.version == "chem1c-v1",
            "M7C model identity changed");
        require(metadata.description
            && metadata.description->find("capacity-one") != std::string::npos,
            "M7C capacity-one competition scope missing");
        require(metadata.validityScope.find("production") != std::string::npos,
            "M7C production limitation missing");
        require(schema.ledger.size() == 1
            && schema.ledger[0].key == "ligand_count_conservation",
            "M7C conservation ledger declaration missing");
        require(schema.regionBindingIdentifier
            && *schema.regionBindingIdentifier == "chem1.concentric_spherical_finite_bath",
            "M7C calibration geometry identifier changed");
        require(contract.referenceModels.empty(), "M7C acquired an unjustified higher-fidelity edge");
        require(contract.description
            && contract.description->find("distinct roles") != std::string::npos,
            "M7C cost-aware ACP role separation missing");
    }
}

int main()
{
    try
    {
        constructionAndModes();
        spectralGoldenValues();
        cdfAndEquilibriumChecks();
        stateAndTrajectory();
        metadataScope();
        std::cout << "CHEM-1C conserved competitive finite-bath tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
