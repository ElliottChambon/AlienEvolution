#include "alien_evolution/physics/ReversibleChemicalAssociationFiniteBath.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <string>

namespace
{
    constexpr double kProbabilitySlack = 2.0e-8;
    constexpr std::size_t kMinimumModes = 16;

    void finiteNonnegative(double value, const char* message)
    {
        if (!std::isfinite(value) || value < 0.0) throw std::invalid_argument(message);
    }

    void finitePositive(double value, const char* message)
    {
        if (!std::isfinite(value) || value <= 0.0) throw std::invalid_argument(message);
    }

    double checkedPositive(double value, const char* message)
    {
        if (!std::isfinite(value) || value <= 0.0) throw std::overflow_error(message);
        return value;
    }

    double stableSquareRatio(double multiplier, double length, double divisor)
    {
        int em = 0, el = 0, ed = 0;
        const auto mm = std::frexp(multiplier, &em);
        const auto ml = std::frexp(length, &el);
        const auto md = std::frexp(divisor, &ed);
        const auto mantissa = mm * ml * ml / md;
        const auto value = std::scalbn(mantissa, em + 2 * el - ed);
        if (!std::isfinite(value) || value < 0.0)
            throw std::overflow_error("CHEM-1C dimensionless/physical timescale is not representable.");
        return value;
    }

    double stableShellVolume(double a, double R)
    {
        const auto difference = R - a;
        const auto quadratic = R * R + R * a + a * a;
        const auto value = (4.0 * std::numbers::pi / 3.0) * difference * quadratic;
        return checkedPositive(value, "CHEM-1C accessible volume is not representable.");
    }

    double drawOpen01(ae::Random& random)
    {
        double value = 0.0;
        do { value = random.uniform01(); } while (value <= 0.0);
        return value;
    }

    double checkedProbability(double value, const char* message)
    {
        if (!std::isfinite(value) || value < -kProbabilitySlack || value > 1.0 + kProbabilitySlack)
            throw std::runtime_error(message);
        return std::clamp(value, 0.0, 1.0);
    }

    double checkedNonnegative(double value, double scale, const char* message)
    {
        if (!std::isfinite(value) || value < -kProbabilitySlack * std::max(1.0, scale))
            throw std::runtime_error(message);
        return std::max(0.0, value);
    }

    double modeFunction(double x, double z, double H)
    {
        return std::cos(z * x) + (H / z) * std::sin(z * x);
    }

    double modeNorm(double ell, double z, double H)
    {
        const auto b = H / z;
        const auto s = std::sin(z * ell);
        const auto sin2 = std::sin(2.0 * z * ell);
        const auto value =
            ell / 2.0
            + sin2 / (4.0 * z)
            + b * s * s / z
            + b * b * (ell / 2.0 - sin2 / (4.0 * z));
        return checkedPositive(value, "CHEM-1C spectral mode norm is not positive/representable.");
    }

    double radialIntegral(double x, double z, double H)
    {
        if (x == 0.0) return 0.0;
        const auto s = std::sin(z * x);
        const auto c = std::cos(z * x);
        const auto invZ = 1.0 / z;
        const auto invZ2 = invZ * invZ;
        const auto cosinePart =
            (1.0 + x) * s * invZ + (c - 1.0) * invZ2;
        const auto sinePart =
            (1.0 - (1.0 + x) * c) * invZ + s * invZ2;
        return cosinePart + (H * invZ) * sinePart;
    }

    double phaseEquation(double z, double ell, double HInner, double HOuter, std::size_t n)
    {
        return z * ell
            - std::atan(HInner / z)
            + std::atan(HOuter / z)
            - static_cast<double>(n) * std::numbers::pi;
    }

    double phaseRoot(double ell, double HInner, double HOuter, std::size_t n)
    {
        double lower = 0.0;
        if (n == 0)
        {
            lower = 1.0e-12 / std::max(1.0, ell);
        }
        else
        {
            lower = static_cast<double>(n) * std::numbers::pi / ell;
        }
        double upper = static_cast<double>(n + 1) * std::numbers::pi / ell;

        auto fLower = phaseEquation(lower, ell, HInner, HOuter, n);
        auto fUpper = phaseEquation(upper, ell, HInner, HOuter, n);
        if (!(fLower < 0.0 && fUpper > 0.0))
            throw std::domain_error("CHEM-1C spectral root could not be bracketed in the approved phase interval.");

        for (int iteration = 0; iteration < 90; ++iteration)
        {
            const auto middle = 0.5 * (lower + upper);
            const auto fMiddle = phaseEquation(middle, ell, HInner, HOuter, n);
            if (fMiddle > 0.0)
                upper = middle;
            else
                lower = middle;
        }
        return 0.5 * (lower + upper);
    }

    ae::PhysicalQuantityDescriptor quantity(
        const char* key,
        const char* unit,
        const char* description)
    {
        return {key, std::string("chem1.finite_bath.") + key, unit, description};
    }

    ae::PhysicalCouplingProcessSchema makeSchema()
    {
        ae::PhysicalCouplingProcessSchema schema;
        schema.identifier = "alien_evolution.chem1.finite_bath_reference.schema";
        schema.model.identifier = "alien_evolution.chem1.finite_bath_reference";
        schema.model.name = "CHEM-1 conserved competitive finite-bath reference";
        schema.model.version = "chem1c-v1";
        schema.model.description =
            "M7C concentric-spherical finite-bath reference: N identical ligands compete for one capacity-one site. "
            "Event-driven radial spectral kernels; not production sensing or a universal spatial chemistry solver.";
        schema.model.parameters = {
            {"a", "m", "Inner encounter radius; calibration geometry, not anatomy"},
            {"R", "m", "Reflecting outer calibration radius"},
            {"D", "m^2/s", "Relative ligand-target diffusion coefficient"},
            {"k_a", "m^3/s", "Intrinsic Collins-Kimball association constant"},
            {"k_d", "1/s", "Intrinsic dissociation rate"},
            {"tau_min", "dimensionless", "Simulator-owned minimum resolved spectral time"},
            {"spectral_tolerance", "dimensionless", "Mode attenuation/truncation control, not a scientific confidence"},
            {"max_modes", "count", "Simulator-owned numerical resource limit"}};
        schema.model.assumptions = {
            "Concentric spherical shell a<r<R with reflecting outer boundary",
            "One capacity-one uniformly reactive inner target; no double binding",
            "When free the inner surface is Robin-reactive; while occupied it is reflecting to competitors",
            "N identical noninteracting point ligands couple only through single-site occupancy",
            "Dissociation uses intrinsic k_d and returns the same ligand to r=a without an arbitrary unbinding radius",
            "Angular coordinates are removed only because the benchmark is exactly spherically symmetric",
            "Spectral propagation below the configured tau_min is numerically unresolved and rejected",
            "No maintained open reservoir, arbitrary geometry, mixtures, crowding, electrostatics or nonideal thermodynamics",
            "Reference/numerical settings are simulator-owned and never inherited or evolvable"};
        schema.model.validityScope =
            "CHEM-1C finite conserved competitive spherical-bath calibration only; "
            "general many-particle spatial chemistry and production PCP integration remain unresolved.";
        schema.model.uncertainties = {
            ae::UncertaintyKind::NumericalReduction,
            ae::UncertaintyKind::ScientificModelForm};
        schema.model.evidence = {{
            ae::EvidenceStatus::EstablishedPhysicalInteraction,
            "Competitive reversible diffusion to a partially reactive target is represented by a bounded Smoluchowski/Robin benchmark.",
            {
                {"Agmon (1993)", "https://doi.org/10.1103/PhysRevE.47.2415",
                    "Competitive single-capacity reversible binding is genuinely many-body"},
                {"Grebenkov (2010)", "https://doi.org/10.1063/1.3294882",
                    "Partially reactive spherical first-reaction theory"},
                {"Prustel and Meier-Schellersheim (2021)", "https://pmc.ncbi.nlm.nih.gov/articles/PMC7940439/",
                    "First-passage reversible binding in spherical confinement"},
                {"Zhang and Isaacson (2022)", "https://pubmed.ncbi.nlm.nih.gov/35649822/",
                    "Reversible spatial detailed-balance/product-placement constraints"}},
            "P1 supports the calibration physics, not universal alien chemistry or biological sensory calibration."}};

        schema.ports = {
            {"finite_bath_ligands",
                quantity("ligand_count", "count", "Conserved total ligand count in the closed calibration bath"),
                ae::PortDirectionality::Bidirectional}};
        schema.state = {
            quantity("binary_bound_state", "dimensionless", "Capacity-one target occupancy B in {0,1}"),
            quantity("free_ligand_radii", "m", "Radial positions of currently unbound conserved ligands")};
        schema.observables = {
            quantity("reactive_survival", "dimensionless", "Single-ligand no-reaction probability in the bounded bath"),
            quantity("first_reaction_density", "1/s", "Single-ligand first-reaction-time density"),
            quantity("equilibrium_bound_probability", "dimensionless", "Exact finite-system capacity-one equilibrium occupancy"),
            quantity("occupancy_trajectory", "dimensionless", "Seeded stochastic finite-bath binding state through time")};
        schema.ledger = {
            quantity("ligand_count_conservation", "count", "N_free + B = N_total exactly at bookkeeping level")};
        schema.regionBindingIdentifier = "chem1.concentric_spherical_finite_bath";
        schema.validate();
        return schema;
    }

    ae::AdaptivePhysicsContract makeContract(const ae::PhysicalCouplingProcessSchema& schema)
    {
        ae::AdaptivePhysicsContract contract{
            {schema.model.identifier, schema.model.version},
            schema.observables,
            {
                {"finite_conserved_bath", "chem1c.closed_concentric_spherical_bath",
                    "Fixed total ligand count; reflecting outer sphere; no source/sink reservoir.", std::nullopt},
                {"single_capacity", "chem1c.capacity_one_competition",
                    "At most one ligand is bound; competitors see a reflecting inner boundary while occupied.", std::nullopt},
                {"spectral_resolution", "chem1c.minimum_resolved_dimensionless_time",
                    "Spectral propagation is only evaluated at or above the configured tau_min with a mode table satisfying the attenuation policy.",
                    "Numerical resolution criterion only; not a scientific validity threshold"}},
            {
                {"survival_reference_difference", "reactive_survival",
                    "chem1c.golden_or_refinement_difference", "dimensionless",
                    "Numerical spectral difference only; model-form uncertainty remains separate."},
                {"first_reaction_density_difference", "first_reaction_density",
                    "chem1c.golden_or_refinement_difference", "1/s",
                    "Numerical spectral difference only."},
                {"equilibrium_probability_difference", "equilibrium_bound_probability",
                    "chem1c.exact_equilibrium_difference", "dimensionless",
                    "Difference from N/(N+K_D V); diffusion may alter kinetics but not the declared equilibrium relation."},
                {"trajectory_cross_fidelity_difference", "occupancy_trajectory",
                    "chem1c.finite_well_mixed_or_future_reference_difference", "dimensionless",
                    "Trajectory/statistical comparison is QoI-specific; no scalar fidelity score."}},
            {},
            "Bounded competitive reference for declared CHEM-1C QoIs. "
            "Reference model, runtime physical model and compiled reduction remain distinct roles; "
            "no automatic ACP model selection is implemented."};
        contract.validate();
        return contract;
    }
}

namespace ae
{
    ReversibleChemicalAssociationFiniteBath::ReversibleChemicalAssociationFiniteBath(
        ChemicalAssociationParameters parameters,
        FiniteChemicalBathConfig config)
        : parameters_(parameters),
          config_(config)
    {
        const auto a = parameters_.encounterRadius();
        finitePositive(config_.outerRadius, "CHEM-1C outer radius must be finite and positive.");
        if (!(config_.outerRadius > a))
            throw std::invalid_argument("CHEM-1C outer radius must exceed encounter radius.");
        finitePositive(config_.spectralTolerance, "CHEM-1C spectral tolerance must be finite and positive.");
        if (!(config_.spectralTolerance < 1.0))
            throw std::invalid_argument("CHEM-1C spectral tolerance must be below one.");
        finitePositive(config_.minimumResolvedDimensionlessTime,
            "CHEM-1C minimum resolved dimensionless time must be finite and positive.");
        if (config_.maxModes < kMinimumModes)
            throw std::invalid_argument("CHEM-1C maxModes is too small for the minimum spectral table.");

        const ReversibleChemicalAssociation reduced(parameters_);
        const auto chi = reduced.rates().intrinsicToDiffusionRatio;
        const auto kd = parameters_.intrinsicDissociationRate();
        const auto delta = kd == 0.0
            ? 0.0
            : stableSquareRatio(kd, a, parameters_.relativeDiffusionCoefficient());
        const auto lambda = config_.outerRadius / a;
        if (!std::isfinite(lambda) || !(lambda > 1.0))
            throw std::overflow_error("CHEM-1C confinement ratio is not representable.");

        const auto volume = stableShellVolume(a, config_.outerRadius);
        const auto minimumPhysicalTime =
            stableSquareRatio(config_.minimumResolvedDimensionlessTime, a,
                parameters_.relativeDiffusionCoefficient());

        diagnostics_ = {
            chi,
            delta,
            lambda,
            volume,
            minimumPhysicalTime,
            0,
            0};

        const auto ell = lambda - 1.0;
        const auto HOuter = 1.0 / lambda;
        const auto attenuationTarget = std::max(
            std::numeric_limits<double>::min(),
            config_.spectralTolerance * 0.1);

        auto buildModes = [&](double HInner, bool reflecting)
        {
            std::vector<SpectralMode> modes;
            const auto start = reflecting ? std::size_t{1} : std::size_t{0};
            for (std::size_t offset = 0; offset < config_.maxModes; ++offset)
            {
                const auto n = start + offset;
                const auto z = phaseRoot(ell, HInner, HOuter, n);
                const auto norm = modeNorm(ell, z, HInner);
                const auto integral = radialIntegral(ell, z, HInner);
                modes.push_back({z, norm, integral});

                if (modes.size() >= kMinimumModes)
                {
                    const auto attenuation =
                        std::exp(-z * z * config_.minimumResolvedDimensionlessTime);
                    if (attenuation <= attenuationTarget)
                        return modes;
                }
            }
            throw std::domain_error(
                "CHEM-1C maxModes cannot satisfy the requested tau_min/spectral-tolerance attenuation policy.");
        };

        reactiveModes_ = buildModes(1.0 + chi, false);
        reflectingModes_ = buildModes(1.0, true);
        diagnostics_.reactiveModeCount = reactiveModes_.size();
        diagnostics_.reflectingModeCount = reflectingModes_.size();
    }

    const ChemicalAssociationParameters&
        ReversibleChemicalAssociationFiniteBath::parameters() const
    {
        return parameters_;
    }

    const FiniteChemicalBathConfig&
        ReversibleChemicalAssociationFiniteBath::config() const
    {
        return config_;
    }

    const FiniteChemicalBathDiagnostics&
        ReversibleChemicalAssociationFiniteBath::diagnostics() const
    {
        return diagnostics_;
    }

    std::vector<double>
        ReversibleChemicalAssociationFiniteBath::reactiveDimensionlessModeRoots() const
    {
        std::vector<double> roots;
        roots.reserve(reactiveModes_.size());
        for (const auto& mode : reactiveModes_) roots.push_back(mode.z);
        return roots;
    }

    std::vector<double>
        ReversibleChemicalAssociationFiniteBath::reflectingDimensionlessModeRoots() const
    {
        std::vector<double> roots;
        roots.reserve(reflectingModes_.size());
        for (const auto& mode : reflectingModes_) roots.push_back(mode.z);
        return roots;
    }

    double ReversibleChemicalAssociationFiniteBath::accessibleVolume() const
    {
        return diagnostics_.accessibleVolume;
    }

    double ReversibleChemicalAssociationFiniteBath::equilibriumBoundProbability(
        std::size_t totalLigands) const
    {
        if (totalLigands == 0) return 0.0;
        if (parameters_.intrinsicDissociationRate() == 0.0) return 1.0;

        const auto Kd =
            parameters_.intrinsicDissociationRate()
            / parameters_.intrinsicAssociationConstant();
        const auto unboundWeight = Kd * diagnostics_.accessibleVolume;
        if (!std::isfinite(unboundWeight))
            return 0.0;

        const auto boundWeight = static_cast<double>(totalLigands);
        if (!std::isfinite(boundWeight))
            throw std::overflow_error("CHEM-1C ligand count cannot be represented as double.");
        return boundWeight / (boundWeight + unboundWeight);
    }

    double ReversibleChemicalAssociationFiniteBath::meanFirstReactionTime(
        double initialRadius) const
    {
        finitePositive(initialRadius, "CHEM-1C initial radius must be finite and positive.");
        const auto a = parameters_.encounterRadius();
        const auto R = config_.outerRadius;
        if (initialRadius < a || initialRadius > R)
            throw std::invalid_argument("CHEM-1C initial radius must lie within [a,R].");

        const auto contact =
            diagnostics_.accessibleVolume / parameters_.intrinsicAssociationConstant();
        const auto D = parameters_.relativeDiffusionCoefficient();
        const auto diffusion =
            (R * R * R / (3.0 * D)) * (1.0 / a - 1.0 / initialRadius)
            - (initialRadius * initialRadius - a * a) / (6.0 * D);
        const auto value = contact + diffusion;
        return checkedPositive(value, "CHEM-1C mean first-reaction time is not representable.");
    }

    double ReversibleChemicalAssociationFiniteBath::dimensionlessTime(
        double elapsedTime) const
    {
        finitePositive(elapsedTime, "CHEM-1C elapsed time must be finite and positive.");
        const auto tau =
            parameters_.relativeDiffusionCoefficient() * elapsedTime
            / (parameters_.encounterRadius() * parameters_.encounterRadius());
        if (!std::isfinite(tau) || tau <= 0.0)
            throw std::overflow_error("CHEM-1C dimensionless time is not representable.");
        if (tau < config_.minimumResolvedDimensionlessTime)
            throw std::domain_error(
                "CHEM-1C requested propagation lies below minimumResolvedDimensionlessTime.");
        return tau;
    }

    double ReversibleChemicalAssociationFiniteBath::reactiveSurvivalDimensionless(
        double rho0,
        double tau) const
    {
        const auto H = 1.0 + diagnostics_.chi;
        const auto x0 = rho0 - 1.0;
        double value = 0.0;
        double scale = 0.0;
        for (const auto& mode : reactiveModes_)
        {
            const auto exponential = std::exp(-mode.z * mode.z * tau);
            const auto coefficient =
                modeFunction(x0, mode.z, H)
                * mode.fullRadialIntegral
                / (rho0 * mode.norm);
            const auto term = coefficient * exponential;
            value += term;
            scale += std::abs(term);
        }
        value = checkedNonnegative(value, scale, "CHEM-1C reactive survival became materially negative.");
        return checkedProbability(value, "CHEM-1C reactive survival left [0,1].");
    }

    double ReversibleChemicalAssociationFiniteBath::reactiveFirstReactionDensityDimensionless(
        double rho0,
        double tau) const
    {
        const auto H = 1.0 + diagnostics_.chi;
        const auto x0 = rho0 - 1.0;
        double value = 0.0;
        double scale = 0.0;
        for (const auto& mode : reactiveModes_)
        {
            const auto exponential = std::exp(-mode.z * mode.z * tau);
            const auto coefficient =
                modeFunction(x0, mode.z, H)
                * mode.fullRadialIntegral
                / (rho0 * mode.norm);
            const auto term = coefficient * mode.z * mode.z * exponential;
            value += term;
            scale += std::abs(term);
        }
        return checkedNonnegative(
            value,
            scale,
            "CHEM-1C first-reaction density became materially negative.");
    }

    double ReversibleChemicalAssociationFiniteBath::reactiveSurvival(
        double initialRadius,
        double elapsedTime) const
    {
        finitePositive(initialRadius, "CHEM-1C initial radius must be finite and positive.");
        const auto a = parameters_.encounterRadius();
        if (initialRadius < a || initialRadius > config_.outerRadius)
            throw std::invalid_argument("CHEM-1C initial radius must lie within [a,R].");
        return reactiveSurvivalDimensionless(initialRadius / a, dimensionlessTime(elapsedTime));
    }

    double ReversibleChemicalAssociationFiniteBath::reactiveFirstReactionDensity(
        double initialRadius,
        double elapsedTime) const
    {
        finitePositive(initialRadius, "CHEM-1C initial radius must be finite and positive.");
        const auto a = parameters_.encounterRadius();
        if (initialRadius < a || initialRadius > config_.outerRadius)
            throw std::invalid_argument("CHEM-1C initial radius must lie within [a,R].");
        const auto tau = dimensionlessTime(elapsedTime);
        const auto dimensionless =
            reactiveFirstReactionDensityDimensionless(initialRadius / a, tau);
        return dimensionless
            * parameters_.relativeDiffusionCoefficient()
            / (a * a);
    }

    double ReversibleChemicalAssociationFiniteBath::reactiveRadialShellProbabilityDensity(
        double radius,
        double initialRadius,
        double elapsedTime) const
    {
        finitePositive(radius, "CHEM-1C radius must be finite and positive.");
        finitePositive(initialRadius, "CHEM-1C initial radius must be finite and positive.");
        const auto a = parameters_.encounterRadius();
        if (radius < a || radius > config_.outerRadius
            || initialRadius < a || initialRadius > config_.outerRadius)
            throw std::invalid_argument("CHEM-1C radii must lie within [a,R].");

        const auto tau = dimensionlessTime(elapsedTime);
        const auto rho = radius / a;
        const auto rho0 = initialRadius / a;
        const auto H = 1.0 + diagnostics_.chi;
        const auto x = rho - 1.0;
        const auto x0 = rho0 - 1.0;
        double sum = 0.0;
        double scale = 0.0;
        for (const auto& mode : reactiveModes_)
        {
            const auto term =
                modeFunction(x, mode.z, H)
                * modeFunction(x0, mode.z, H)
                / mode.norm
                * std::exp(-mode.z * mode.z * tau);
            sum += term;
            scale += std::abs(term);
        }
        const auto dimensionless = (rho / rho0)
            * checkedNonnegative(sum, scale, "CHEM-1C reactive radial density became materially negative.");
        const auto value = dimensionless / a;
        if (!std::isfinite(value) || value < 0.0)
            throw std::overflow_error("CHEM-1C reactive radial density is not representable.");
        return value;
    }

    double ReversibleChemicalAssociationFiniteBath::reactiveConditionalCdfDimensionless(
        double rho,
        double rho0,
        double tau) const
    {
        if (rho <= 1.0) return 0.0;
        if (rho >= diagnostics_.lambda) return 1.0;

        const auto H = 1.0 + diagnostics_.chi;
        const auto x = rho - 1.0;
        const auto x0 = rho0 - 1.0;
        double numerator = 0.0;
        for (const auto& mode : reactiveModes_)
        {
            numerator +=
                modeFunction(x0, mode.z, H)
                * radialIntegral(x, mode.z, H)
                / (rho0 * mode.norm)
                * std::exp(-mode.z * mode.z * tau);
        }
        const auto survival = reactiveSurvivalDimensionless(rho0, tau);
        if (!(survival > 0.0))
            throw std::domain_error("CHEM-1C conditional radial CDF is undefined after survival underflow.");
        return checkedProbability(
            numerator / survival,
            "CHEM-1C conditional reactive radial CDF left [0,1].");
    }

    double ReversibleChemicalAssociationFiniteBath::reactiveConditionalRadialCdf(
        double radius,
        double initialRadius,
        double elapsedTime) const
    {
        finitePositive(radius, "CHEM-1C radius must be finite and positive.");
        finitePositive(initialRadius, "CHEM-1C initial radius must be finite and positive.");
        const auto a = parameters_.encounterRadius();
        if (radius < a || radius > config_.outerRadius
            || initialRadius < a || initialRadius > config_.outerRadius)
            throw std::invalid_argument("CHEM-1C radii must lie within [a,R].");
        return reactiveConditionalCdfDimensionless(
            radius / a,
            initialRadius / a,
            dimensionlessTime(elapsedTime));
    }

    double ReversibleChemicalAssociationFiniteBath::reflectingRadialShellProbabilityDensity(
        double radius,
        double initialRadius,
        double elapsedTime) const
    {
        finitePositive(radius, "CHEM-1C radius must be finite and positive.");
        finitePositive(initialRadius, "CHEM-1C initial radius must be finite and positive.");
        const auto a = parameters_.encounterRadius();
        if (radius < a || radius > config_.outerRadius
            || initialRadius < a || initialRadius > config_.outerRadius)
            throw std::invalid_argument("CHEM-1C radii must lie within [a,R].");

        const auto tau = dimensionlessTime(elapsedTime);
        const auto rho = radius / a;
        const auto rho0 = initialRadius / a;
        const auto x = rho - 1.0;
        const auto x0 = rho0 - 1.0;
        const auto volumeDenominator =
            diagnostics_.lambda * diagnostics_.lambda * diagnostics_.lambda - 1.0;

        double dimensionless = 3.0 * rho * rho / volumeDenominator;
        double scale = std::abs(dimensionless);
        for (const auto& mode : reflectingModes_)
        {
            const auto term =
                (rho / rho0)
                * modeFunction(x, mode.z, 1.0)
                * modeFunction(x0, mode.z, 1.0)
                / mode.norm
                * std::exp(-mode.z * mode.z * tau);
            dimensionless += term;
            scale += std::abs(term);
        }
        dimensionless = checkedNonnegative(
            dimensionless,
            scale,
            "CHEM-1C reflecting radial density became materially negative.");
        const auto value = dimensionless / a;
        if (!std::isfinite(value) || value < 0.0)
            throw std::overflow_error("CHEM-1C reflecting radial density is not representable.");
        return value;
    }

    double ReversibleChemicalAssociationFiniteBath::reflectingCdfDimensionless(
        double rho,
        double rho0,
        double tau) const
    {
        if (rho <= 1.0) return 0.0;
        if (rho >= diagnostics_.lambda) return 1.0;

        const auto x = rho - 1.0;
        const auto x0 = rho0 - 1.0;
        const auto denominator =
            diagnostics_.lambda * diagnostics_.lambda * diagnostics_.lambda - 1.0;
        double value = (rho * rho * rho - 1.0) / denominator;

        for (const auto& mode : reflectingModes_)
        {
            value +=
                modeFunction(x0, mode.z, 1.0)
                * radialIntegral(x, mode.z, 1.0)
                / (rho0 * mode.norm)
                * std::exp(-mode.z * mode.z * tau);
        }
        return checkedProbability(value, "CHEM-1C reflecting radial CDF left [0,1].");
    }

    double ReversibleChemicalAssociationFiniteBath::reflectingRadialCdf(
        double radius,
        double initialRadius,
        double elapsedTime) const
    {
        finitePositive(radius, "CHEM-1C radius must be finite and positive.");
        finitePositive(initialRadius, "CHEM-1C initial radius must be finite and positive.");
        const auto a = parameters_.encounterRadius();
        if (radius < a || radius > config_.outerRadius
            || initialRadius < a || initialRadius > config_.outerRadius)
            throw std::invalid_argument("CHEM-1C radii must lie within [a,R].");
        return reflectingCdfDimensionless(
            radius / a,
            initialRadius / a,
            dimensionlessTime(elapsedTime));
    }

    void ReversibleChemicalAssociationFiniteBath::validateState(
        const FiniteChemicalBathState& state) const
    {
        const auto a = parameters_.encounterRadius();
        for (std::size_t index = 0; index < state.ligandRadii.size(); ++index)
        {
            const auto radius = state.ligandRadii[index];
            if (!std::isfinite(radius) || radius < a || radius > config_.outerRadius)
                throw std::invalid_argument("CHEM-1C ligand radius must lie within [a,R].");
        }
        if (state.boundLigand)
        {
            if (*state.boundLigand >= state.ligandRadii.size())
                throw std::invalid_argument("CHEM-1C bound ligand index is out of range.");
            if (state.ligandRadii[*state.boundLigand] != a)
                throw std::invalid_argument(
                    "CHEM-1C bound ligand must be stored exactly at encounter radius a.");
        }
    }

    double ReversibleChemicalAssociationFiniteBath::sampleReactiveConditionalRadius(
        double initialRadius,
        double elapsedTime,
        Random& random) const
    {
        const auto target = random.uniform01();
        double lower = parameters_.encounterRadius();
        double upper = config_.outerRadius;
        for (int iteration = 0; iteration < 64; ++iteration)
        {
            const auto middle = 0.5 * (lower + upper);
            const auto cdf = reactiveConditionalRadialCdf(middle, initialRadius, elapsedTime);
            if (cdf < target)
                lower = middle;
            else
                upper = middle;
        }
        return 0.5 * (lower + upper);
    }

    double ReversibleChemicalAssociationFiniteBath::sampleReflectingRadius(
        double initialRadius,
        double elapsedTime,
        Random& random) const
    {
        const auto target = random.uniform01();
        double lower = parameters_.encounterRadius();
        double upper = config_.outerRadius;
        for (int iteration = 0; iteration < 64; ++iteration)
        {
            const auto middle = 0.5 * (lower + upper);
            const auto cdf = reflectingRadialCdf(middle, initialRadius, elapsedTime);
            if (cdf < target)
                lower = middle;
            else
                upper = middle;
        }
        return 0.5 * (lower + upper);
    }

    FiniteChemicalBathTrajectory
        ReversibleChemicalAssociationFiniteBath::sampleTrajectory(
            const FiniteChemicalBathState& initialState,
            double duration,
            Random& random,
            std::size_t eventLimit) const
    {
        finiteNonnegative(duration, "CHEM-1C trajectory duration must be finite and nonnegative.");
        validateState(initialState);

        FiniteChemicalBathTrajectory result{
            initialState,
            initialState,
            duration,
            {}};
        if (duration == 0.0) return result;

        const auto minimumTime = diagnostics_.minimumResolvedPhysicalTime;
        const auto a = parameters_.encounterRadius();
        const auto kd = parameters_.intrinsicDissociationRate();
        double time = 0.0;

        auto propagateReactiveSurvivors = [&](double interval, std::optional<std::size_t> excluded)
        {
            if (interval < minimumTime)
                throw std::domain_error(
                    "CHEM-1C reactive propagation interval lies below the configured spectral resolution.");
            for (std::size_t i = 0; i < result.finalState.ligandRadii.size(); ++i)
            {
                if (excluded && i == *excluded) continue;
                result.finalState.ligandRadii[i] =
                    sampleReactiveConditionalRadius(
                        result.finalState.ligandRadii[i],
                        interval,
                        random);
            }
        };

        auto propagateReflectingCompetitors = [&](double interval, std::size_t bound)
        {
            if (result.finalState.ligandRadii.size() <= 1) return;
            if (interval < minimumTime)
                throw std::domain_error(
                    "CHEM-1C reflecting propagation interval lies below the configured spectral resolution.");
            for (std::size_t i = 0; i < result.finalState.ligandRadii.size(); ++i)
            {
                if (i == bound) continue;
                result.finalState.ligandRadii[i] =
                    sampleReflectingRadius(
                        result.finalState.ligandRadii[i],
                        interval,
                        random);
            }
        };

        while (time < duration)
        {
            const auto remaining = duration - time;

            if (result.finalState.boundLigand)
            {
                const auto bound = *result.finalState.boundLigand;
                if (kd == 0.0)
                {
                    propagateReflectingCompetitors(remaining, bound);
                    time = duration;
                    break;
                }

                const auto wait = -std::log1p(-drawOpen01(random)) / kd;
                if (!std::isfinite(wait))
                    throw std::overflow_error("CHEM-1C dissociation waiting time is not representable.");

                if (wait >= remaining)
                {
                    propagateReflectingCompetitors(remaining, bound);
                    time = duration;
                    break;
                }

                propagateReflectingCompetitors(wait, bound);
                time += wait;
                result.finalState.ligandRadii[bound] = a;
                result.finalState.boundLigand.reset();
                if (result.events.size() >= eventLimit)
                    throw std::runtime_error("CHEM-1C event safety limit exceeded.");
                result.events.push_back({time, FiniteChemicalBathEventKind::Dissociation, bound});
                continue;
            }

            const auto ligandCount = result.finalState.ligandRadii.size();
            if (ligandCount == 0)
            {
                time = duration;
                break;
            }
            if (remaining < minimumTime)
                throw std::domain_error(
                    "CHEM-1C remaining free-site interval lies below the configured spectral resolution.");

            const auto logTarget = std::log(drawOpen01(random));
            auto totalLogSurvival = [&](double interval)
            {
                double total = 0.0;
                for (const auto radius : result.finalState.ligandRadii)
                {
                    const auto survival = reactiveSurvival(radius, interval);
                    if (!(survival > 0.0)) return -std::numeric_limits<double>::infinity();
                    total += std::log(survival);
                }
                return total;
            };

            const auto logAtMinimum = totalLogSurvival(minimumTime);
            if (logTarget > logAtMinimum)
                throw std::domain_error(
                    "CHEM-1C sampled a binding event below minimumResolvedDimensionlessTime.");

            const auto logAtEnd = totalLogSurvival(remaining);
            if (logTarget <= logAtEnd)
            {
                propagateReactiveSurvivors(remaining, std::nullopt);
                time = duration;
                break;
            }

            double lower = minimumTime;
            double upper = remaining;
            for (int iteration = 0; iteration < 64; ++iteration)
            {
                const auto middle = 0.5 * (lower + upper);
                if (totalLogSurvival(middle) > logTarget)
                    lower = middle;
                else
                    upper = middle;
            }
            const auto eventInterval = 0.5 * (lower + upper);

            std::vector<double> weights(ligandCount, 0.0);
            double totalWeight = 0.0;
            for (std::size_t i = 0; i < ligandCount; ++i)
            {
                const auto radius = result.finalState.ligandRadii[i];
                const auto survival = reactiveSurvival(radius, eventInterval);
                const auto density = reactiveFirstReactionDensity(radius, eventInterval);
                if (!(survival > 0.0))
                    throw std::domain_error("CHEM-1C winner hazard is undefined after survival underflow.");
                weights[i] = density / survival;
                totalWeight += weights[i];
            }
            if (!std::isfinite(totalWeight) || !(totalWeight > 0.0))
                throw std::runtime_error("CHEM-1C total binding hazard is not positive/representable.");

            const auto winnerDraw = random.uniform01() * totalWeight;
            double cumulative = 0.0;
            std::size_t winner = ligandCount - 1;
            for (std::size_t i = 0; i < ligandCount; ++i)
            {
                cumulative += weights[i];
                if (winnerDraw < cumulative)
                {
                    winner = i;
                    break;
                }
            }

            propagateReactiveSurvivors(eventInterval, winner);
            result.finalState.ligandRadii[winner] = a;
            result.finalState.boundLigand = winner;
            time += eventInterval;

            if (result.events.size() >= eventLimit)
                throw std::runtime_error("CHEM-1C event safety limit exceeded.");
            result.events.push_back({time, FiniteChemicalBathEventKind::Binding, winner});
        }

        validateState(result.finalState);
        return result;
    }

    const PhysicalCouplingProcessSchema&
        ReversibleChemicalAssociationFiniteBath::schema()
    {
        static const auto value = makeSchema();
        return value;
    }

    const ScientificModelMetadata&
        ReversibleChemicalAssociationFiniteBath::scientificMetadata()
    {
        return schema().model;
    }

    const AdaptivePhysicsContract&
        ReversibleChemicalAssociationFiniteBath::adaptivePhysicsContract()
    {
        static const auto value = makeContract(schema());
        return value;
    }
} // namespace ae
