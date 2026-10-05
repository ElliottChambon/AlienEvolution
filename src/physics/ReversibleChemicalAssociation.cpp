#include "alien_evolution/physics/ReversibleChemicalAssociation.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace
{
    void nonnegative(double value, const char* message)
    {
        if (!std::isfinite(value) || value < 0.0) throw std::invalid_argument(message);
    }
    void positive(double value, const char* message)
    {
        if (!std::isfinite(value) || value <= 0.0) throw std::invalid_argument(message);
    }
    double representablePositive(double value)
    {
        if (!std::isfinite(value) || value <= 0.0)
            throw std::overflow_error("CHEM-1 positive derived quantity is outside representable double range.");
        return value;
    }
    void occupancy(double value)
    {
        nonnegative(value, "Occupancy must be finite and in [0,1].");
        if (value > 1.0) throw std::invalid_argument("Occupancy must be in [0,1].");
    }
    double fraction(double numerator, double other)
    {
        if (numerator == 0.0) return 0.0;
        if (other == 0.0) return 1.0;
        if (numerator >= other) return 1.0 / (1.0 + other / numerator);
        const auto ratio = numerator / other;
        return ratio / (1.0 + ratio);
    }
    ae::ChemicalAssociationRates compileRates(const ae::ChemicalAssociationParameters& p)
    {
        // Avoid avoidable intermediate D*a overflow/underflow; final range errors
        // remain explicit. Probability complements are computed independently.
        int diffusionExponent = 0, radiusExponent = 0;
        const auto dm = std::frexp(p.relativeDiffusionCoefficient(), &diffusionExponent);
        const auto am = std::frexp(p.encounterRadius(), &radiusExponent);
        const auto kD = representablePositive(std::scalbn(4.0 * std::numbers::pi * dm * am,
            diffusionExponent + radiusExponent));
        const auto ka = p.intrinsicAssociationConstant();
        const auto kd = p.intrinsicDissociationRate();
        const auto smaller = std::min(ka, kD);
        const auto larger = std::max(ka, kD);
        const auto on = representablePositive(smaller / (1.0 + smaller / larger));
        const auto off = kd == 0.0 ? 0.0 : representablePositive(kd * fraction(kD, ka));
        const auto dissociation = kd == 0.0 ? 0.0 : representablePositive(kd / ka);
        return {kD, representablePositive(ka / kD), on, fraction(ka, kD), off, dissociation};
    }
}

namespace ae
{
    ChemicalAssociationParameters::ChemicalAssociationParameters(double a, double D, double ka, double kd)
        : radius_(a), diffusion_(D), association_(ka), dissociation_(kd)
    {
        positive(a, "Encounter radius must be finite and positive.");
        positive(D, "Relative diffusion coefficient must be finite and positive.");
        positive(ka, "Intrinsic association constant must be finite and positive.");
        nonnegative(kd, "Intrinsic dissociation rate must be finite and nonnegative.");
    }
    double ChemicalAssociationParameters::encounterRadius() const { return radius_; }
    double ChemicalAssociationParameters::relativeDiffusionCoefficient() const { return diffusion_; }
    double ChemicalAssociationParameters::intrinsicAssociationConstant() const { return association_; }
    double ChemicalAssociationParameters::intrinsicDissociationRate() const { return dissociation_; }

    ReversibleChemicalAssociation::ReversibleChemicalAssociation(ChemicalAssociationParameters parameters)
        : parameters_(parameters), rates_(compileRates(parameters))
    {}
    const ChemicalAssociationParameters& ReversibleChemicalAssociation::parameters() const { return parameters_; }
    const ChemicalAssociationRates& ReversibleChemicalAssociation::rates() const { return rates_; }

    double ReversibleChemicalAssociation::associationHazard(double concentration) const
    {
        nonnegative(concentration, "Ideal concentration must be finite and nonnegative.");
        if (concentration == 0.0) return 0.0;
        return representablePositive(rates_.effectiveAssociationRate * concentration);
    }
    double ReversibleChemicalAssociation::equilibriumOccupancy(double concentration) const
    {
        nonnegative(concentration, "Ideal concentration must be finite and nonnegative.");
        if (concentration == 0.0 && rates_.dissociationConcentration == 0.0)
            throw std::domain_error("At c=0 and k_d=0 there is no unique equilibrium occupancy.");
        return fraction(concentration, rates_.dissociationConcentration);
    }
    double ReversibleChemicalAssociation::relaxationTime(double concentration) const
    {
        const auto on = associationHazard(concentration);
        const auto off = rates_.effectiveDissociationRate;
        const auto maximum = std::max(on, off);
        if (maximum == 0.0) return std::numeric_limits<double>::infinity();
        // Avoid overflow of on+off without imposing a numerical threshold.
        return (1.0 / (1.0 + std::min(on, off) / maximum)) / maximum;
    }
    double ReversibleChemicalAssociation::occupancyDerivative(double b, double concentration) const
    {
        occupancy(b);
        const auto on = associationHazard(concentration);
        return on * (1.0 - b) - rates_.effectiveDissociationRate * b;
    }
    double ReversibleChemicalAssociation::occupancyAfter(double b, double concentration, double time) const
    {
        occupancy(b);
        nonnegative(time, "Elapsed time must be finite and nonnegative.");
        const auto on = associationHazard(concentration);
        const auto off = rates_.effectiveDissociationRate;
        if (time == 0.0 || (on == 0.0 && off == 0.0)) return b;
        // expm1 preserves short-time response. An infinite exponent means the
        // finite-double long-time limit, not a new physical transition rule.
        const auto responded = -std::expm1(-(on * time + off * time));
        return b * (1.0 - responded) + fraction(on, off) * responded;
    }
    ChemicalAssociationTrajectory ReversibleChemicalAssociation::sampleTrajectory(bool initial, double concentration,
        double duration, Random& random, std::size_t eventLimit) const
    {
        nonnegative(duration, "Trajectory duration must be finite and nonnegative.");
        const auto on = associationHazard(concentration);
        const auto off = rates_.effectiveDissociationRate;
        ChemicalAssociationTrajectory result{initial, initial, duration, {}};
        double time = 0.0;
        while (time < duration)
        {
            const auto hazard = result.finallyBound ? off : on;
            if (hazard == 0.0) break;
            double uniform = 0.0;
            do { uniform = random.uniform01(); } while (uniform == 0.0);
            const auto wait = -std::log1p(-uniform) / hazard;
            const auto next = time + wait;
            if (next > duration) break; // includes +infinity (finite horizon).
            if (!(next > time)) throw std::overflow_error("CTMC time increment is not representable.");
            if (result.events.size() >= eventLimit) throw std::runtime_error("CTMC event safety limit exceeded.");
            time = next;
            result.finallyBound = !result.finallyBound;
            result.events.push_back({time, result.finallyBound});
        }
        return result;
    }
} // namespace ae

namespace
{
    using Representation = ae::ChemicalAssociationRepresentation;
    constexpr std::array<const char*, 4> names{
        "analytical", "equilibrium", "well_mixed_deterministic", "well_mixed_ctmc"};
    std::size_t index(Representation representation)
    {
        switch (representation)
        {
        case Representation::AnalyticalRelations: return 0;
        case Representation::EquilibriumReduction: return 1;
        case Representation::DeterministicWellMixed: return 2;
        case Representation::StochasticWellMixed: return 3;
        }
        throw std::invalid_argument("Unknown CHEM-1 representation.");
    }
    ae::PhysicalQuantityDescriptor quantity(const char* key, const char* unit, const char* description)
    {
        return {key, std::string("chem1.") + key, unit, description};
    }
    ae::PhysicalCouplingProcessSchema makeChemicalSchema(std::size_t profile)
    {
        ae::PhysicalCouplingProcessSchema s;
        auto& m = s.model;
        m.identifier = std::string("alien_evolution.chem1.") + names[profile];
        m.version = "chem1a-v1";
        m.name = std::string("CHEM-1 ideal reversible association: ") + names[profile];
        m.description = "M7A benchmark scaffold for L+S <-> LS, not mature chemical sensing. "
            "Provenance: GitHub issue #35 and docs/CHEMICAL_COUPLING_ARCHITECTURE.md. "
            "Compiled effective rates are not elementary chemistry or heritable parameters.";
        m.parameters = {{"a", "m", "Ideal spherical encounter radius, not anatomy"},
            {"D", "m^2/s", "Relative diffusion coefficient"},
            {"k_a", "m^3/s", "Intrinsic association constant in number-based concentration units"},
            {"k_d", "1/s", "Intrinsic dissociation rate"}};
        m.evidence = {{ae::EvidenceStatus::EstablishedPhysicalInteraction,
            "Reversible association is a physical interaction; these relations assume the ideal spherical reduction.",
            {{"Kim and Shin (1999)", "https://doi.org/10.1103/PhysRevLett.82.1578", "Approved CHEM-1 literature context"},
             {"Andrews and Bray (2004)", "https://doi.org/10.1088/1478-3967/1/3/001", "Spatial methods remain deferred"},
             {"van Zon and ten Wolde (2005)", "https://doi.org/10.1103/PhysRevLett.94.128103", "Spatial methods remain deferred"}},
            "P1 is not evidence of biological sensory use, calibration, or spatial cross-validation."}};
        m.assumptions = {"Ideal dilute 3D diffusion to a finite-reactivity sphere; static material support",
            "Generic mobile L and reactive material state S form LS; noncooperative single-site benchmark",
            "Consistent number-based units; concentration is not universal activity",
            "External reservoir, constant concentration per kinetic segment; no ligand depletion",
            "Passive reversible association; no explicit energetic driving or later chemical mechanisms",
            "Spatial solver unselected; rebinding collapsed into effective rates"};
        if (profile == 1) m.assumptions.push_back("Equilibrium only; kinetic history discarded");
        if (profile == 2) m.assumptions.push_back("Well-mixed mean occupancy; intrinsic fluctuations discarded");
        if (profile == 3) m.assumptions.push_back("Well-mixed two-state Markov jump model; spatial history discarded");
        m.validityScope = "Analytical and well-mixed CHEM-1A only under declared assumptions; "
            "no spatial certification, biological sensing claim, or runtime validity evaluation.";
        m.uncertainties = {ae::UncertaintyKind::NumericalReduction, ae::UncertaintyKind::ScientificModelForm};
        s.identifier = m.identifier + ".schema";
        s.description = "Model-owned declarations, not runtime ports or a universal chemistry catalog";
        s.regionBindingIdentifier = "chem1.ideal_spherical_support";
        s.ports = {{"reservoir_concentration", quantity("concentration", "1/m^3",
            "Ideal dilute external reservoir number concentration, not universal activity"), ae::PortDirectionality::Input}};
        s.observables = {quantity("k_D", "m^3/s", "Diffusion-limited association constant"),
            quantity("chi", "dimensionless", "Intrinsic/diffusive ratio, not a fidelity rank"),
            quantity("k_on", "m^3/s", "Compiled effective association constant"),
            quantity("p_rebind", "dimensionless", "Ideal collapsed rebinding probability"),
            quantity("k_off", "1/s", "Compiled effective dissociation rate"),
            quantity("K_D", "1/m^3", "Dissociation concentration, not a universal activity constant")};
        if (profile != 0)
            s.observables.push_back(quantity("equilibrium_occupancy", "dimensionless", "Bound probability at equilibrium"));
        if (profile >= 2)
        {
            s.observables.push_back(quantity("relaxation_time", "s", "Constant-reservoir relaxation time"));
            s.observables.push_back(quantity("occupancy_response", "dimensionless", "Transient mean bound probability"));
            s.state = {quantity(profile == 3 ? "binary_bound_state" : "bound_probability", "dimensionless",
                profile == 3 ? "Unbound or bound material state" : "Mean probability of associated material state")};
            s.ledger = {quantity("probability_normalization", "dimensionless",
                "Unbound plus bound probability equals one; not closed-domain matter conservation"),
                quantity("net_binding_probability_flux", "1/s", "Association minus dissociation probability flux")};
        }
        if (profile == 3)
        {
            s.observables.push_back(quantity("occupancy_mean", "dimensionless", "Ensemble mean binary occupancy"));
            s.observables.push_back(quantity("occupancy_variance", "dimensionless", "Intrinsic Bernoulli occupancy variance"));
        }
        s.validate();
        return s;
    }
    ae::AdaptivePhysicsContract makeChemicalContract(const ae::PhysicalCouplingProcessSchema& s, std::size_t profile)
    {
        ae::AdaptivePhysicsContract c{{s.model.identifier, s.model.version}, s.observables, {}, {}, {},
            "Multidimensional profile: spatial=collapsed ideal 3D sphere; thermodynamic=ideal dilute passive; "
            "chemical detail=single reversible pair; driving=none. Spatial reference UNRESOLVED. "
            "No scalar fidelity rank, solver selection, validity evaluation, or error calculation."};
        c.description = *c.description + (profile == 0 ? " Stochastic/kinetic-history axes: analytical relations only." :
            profile == 1 ? " Stochastic=mean equilibrium; kinetic-history=discarded." :
            profile == 2 ? " Stochastic=mean field; kinetic-history=constant-reservoir Markov relaxation." :
            " Stochastic=intrinsic two-state jumps; kinetic-history=constant-reservoir Markov trajectory.");
        c.validityCriteria = {{"ideal_support", "chem1.ideal_dilute_sphere",
            "Ideal dilute 3D spherical encounter geometry, static single-site material, passive association; "
            "not DiffusiveField2D, crowding, mixtures or driven chemistry.", "Declaration only; spatial check UNRESOLVED"},
            {"reservoir", "chem1.open_constant_reservoir",
            "No depletion; kinetic segments have constant concentration. Well-mixed models collapse rebinding history.",
            "Closed-domain matter conservation and non-Markov spatial history are deferred"}};
        for (const auto& q : c.quantitiesOfInterest)
            c.errorMeasures.push_back({q.key + "_difference", q.key, "chem1.quantity_specific_reference_difference", q.unit,
                "Numerical/reference difference declaration only, no estimator; spatial comparison UNRESOLVED. "
                "Scientific/model-form uncertainty is separate from numerical error and intrinsic stochasticity."});
        // The future spatial obligation spans this contract's QoIs. Do not add
        // an analytical-only contract as an all-QoI reference for trajectory QoIs.
        c.referenceModels.emplace_back("alien_evolution.chem1.spatial_reference", "review-pending");
        c.validate();
        return c;
    }
}

namespace ae
{
    const PhysicalCouplingProcessSchema& ReversibleChemicalAssociation::schema(Representation representation)
    {
        static const std::array descriptors{makeChemicalSchema(0), makeChemicalSchema(1),
            makeChemicalSchema(2), makeChemicalSchema(3)};
        return descriptors[index(representation)];
    }
    const ScientificModelMetadata& ReversibleChemicalAssociation::scientificMetadata(Representation representation)
    {
        return schema(representation).model;
    }
    const AdaptivePhysicsContract& ReversibleChemicalAssociation::adaptivePhysicsContract(Representation representation)
    {
        static const std::array contracts{
            makeChemicalContract(schema(Representation::AnalyticalRelations), 0),
            makeChemicalContract(schema(Representation::EquilibriumReduction), 1),
            makeChemicalContract(schema(Representation::DeterministicWellMixed), 2),
            makeChemicalContract(schema(Representation::StochasticWellMixed), 3)};
        return contracts[index(representation)];
    }
    PhysicsChallengeRegistry ReversibleChemicalAssociation::spatialReferenceChallenges()
    {
        PhysicsChallengeRegistry registry;
        const auto& contract = adaptivePhysicsContract(Representation::StochasticWellMixed);
        const std::array<const char*, 8> cases{"reaction_limited", "diffusion_limited", "transient_forcing",
            "low_copy_stochastic", "spatial_rebinding", "finite_domain_conservation", "geometry_variation", "validity_boundary"};
        const std::array<const char*, 8> contexts{"chi=0.01", "chi=100", "piecewise_constant_reservoir",
            "one_site;seed=3501", "post_dissociation_spatial_history", "closed_domain;finite_L",
            "nonspherical_support", "dilute_to_nondilute_cross_fidelity"};
        for (std::size_t i = 0; i < cases.size(); ++i)
        {
            PhysicsChallengeCase challenge{std::string("chem1.") + cases[i], contract.model,
                {i < 2 ? "k_on" : i == 3 ? "occupancy_variance" : "occupancy_response"},
                {FidelityAuditReason::DeclaredValidityConcern, FidelityAuditReason::CrossFidelityDisagreement},
                {{"alien_evolution.chem1.spatial_reference", "review-pending"}},
                std::string("chem1.challenge:") + contexts[i] + ";spatial_solver=unselected",
                std::string("UNRESOLVED spatial cross-validation: ") + cases[i] +
                    ". Analytical/well-mixed checks do not mark this spatial challenge passed."};
            challenge.validate(contract);
            registry.add(challenge);
        }
        return registry;
    }
}
