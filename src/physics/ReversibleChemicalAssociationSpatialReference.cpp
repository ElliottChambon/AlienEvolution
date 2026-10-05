#include "alien_evolution/physics/ReversibleChemicalAssociationSpatialReference.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <string>

namespace
{
    using Complex = std::complex<double>;

    constexpr double kSqrtPi = 1.7724538509055160272981674833411451828;
    constexpr double kRootSeparationFloor = 1.0e-12;
    constexpr double kImaginaryResidueRelativeTolerance = 5.0e-11;
    constexpr double kNegativeCancellationRelativeTolerance = 5.0e-11;

    void positive(double value, const char* message)
    {
        if (!std::isfinite(value) || value <= 0.0) throw std::invalid_argument(message);
    }

    void nonnegative(double value, const char* message)
    {
        if (!std::isfinite(value) || value < 0.0) throw std::invalid_argument(message);
    }

    double representablePositive(double value, const char* message)
    {
        if (!std::isfinite(value) || value <= 0.0) throw std::overflow_error(message);
        return value;
    }

    double scaledProductRatio(double first, double second, double secondAgain, double denominator)
    {
        if (first == 0.0) return 0.0;
        int firstExponent = 0;
        int secondExponent = 0;
        int thirdExponent = 0;
        int denominatorExponent = 0;
        const auto firstMantissa = std::frexp(first, &firstExponent);
        const auto secondMantissa = std::frexp(second, &secondExponent);
        const auto thirdMantissa = std::frexp(secondAgain, &thirdExponent);
        const auto denominatorMantissa = std::frexp(denominator, &denominatorExponent);
        const auto mantissa = firstMantissa * secondMantissa * thirdMantissa / denominatorMantissa;
        const auto value = std::scalbn(
            mantissa,
            firstExponent + secondExponent + thirdExponent - denominatorExponent);
        return representablePositive(
            value,
            "CHEM-1B dimensionless dissociation ratio is outside representable double range.");
    }

    Complex complexCubeRoot(Complex value)
    {
        if (std::abs(value) == 0.0) return {};
        return std::polar(
            std::cbrt(std::abs(value)),
            std::arg(value) / 3.0);
    }

    std::array<Complex, 3> solveDimensionlessRoots(double chi, double delta)
    {
        // lambda^3 -(1+chi)lambda^2 + delta*lambda - delta = 0.
        const double a = -(1.0 + chi);
        const double b = delta;
        const double c = -delta;

        const Complex p = b - a * a / 3.0;
        const Complex q = 2.0 * a * a * a / 27.0 - a * b / 3.0 + c;
        const Complex discriminant = q * q / 4.0 + p * p * p / 27.0;
        const Complex squareRoot = std::sqrt(discriminant);

        Complex u = complexCubeRoot(-q / 2.0 + squareRoot);
        Complex v{};
        if (std::abs(u) > 16.0 * std::numeric_limits<double>::epsilon())
        {
            v = -p / (3.0 * u);
        }
        else
        {
            v = complexCubeRoot(-q / 2.0 - squareRoot);
            if (std::abs(v) > 16.0 * std::numeric_limits<double>::epsilon())
                u = -p / (3.0 * v);
        }

        const Complex omega{-0.5, std::sqrt(3.0) / 2.0};
        const Complex omegaSquared = std::conj(omega);
        std::array<Complex, 3> roots{
            u + v - a / 3.0,
            omega * u + omegaSquared * v - a / 3.0,
            omegaSquared * u + omega * v - a / 3.0};

        // A few Newton iterations reduce ordinary Cardano roundoff without
        // changing the mathematical model.
        for (auto& root : roots)
        {
            for (int iteration = 0; iteration < 3; ++iteration)
            {
                const auto residual =
                    root * root * root
                    - (1.0 + chi) * root * root
                    + delta * root
                    - delta;
                const auto derivative =
                    3.0 * root * root
                    - 2.0 * (1.0 + chi) * root
                    + delta;
                if (std::abs(derivative) > 32.0 * std::numeric_limits<double>::epsilon())
                    root -= residual / derivative;
            }
        }

        std::sort(
            roots.begin(),
            roots.end(),
            [](const Complex& left, const Complex& right)
            {
                if (left.real() != right.real()) return left.real() < right.real();
                return left.imag() < right.imag();
            });
        return roots;
    }

    double relativeRootSeparation(const std::array<Complex, 3>& roots)
    {
        const auto scale = std::max({
            1.0,
            std::abs(roots[0]),
            std::abs(roots[1]),
            std::abs(roots[2])});
        double minimum = std::numeric_limits<double>::infinity();
        for (std::size_t i = 0; i < roots.size(); ++i)
            for (std::size_t j = i + 1; j < roots.size(); ++j)
                minimum = std::min(minimum, std::abs(roots[i] - roots[j]));
        return minimum / scale;
    }

    Complex erfcxAsymptotic(Complex z)
    {
        Complex term{1.0, 0.0};
        Complex sum = term;
        double previousMagnitude = std::abs(term);
        const Complex inverseTwoZSquared = Complex{1.0, 0.0} / (2.0 * z * z);

        for (int n = 1; n < 80; ++n)
        {
            term *= -(2.0 * n - 1.0) * inverseTwoZSquared;
            const auto magnitude = std::abs(term);
            if (magnitude > previousMagnitude) break;
            sum += term;
            if (magnitude <= 2.0e-16 * std::max(1.0, std::abs(sum))) break;
            previousMagnitude = magnitude;
        }
        return sum / (kSqrtPi * z);
    }

    template <class Function>
    Complex adaptiveSimpson(
        const Function& function,
        double lower,
        double upper,
        Complex fLower,
        Complex fMiddle,
        Complex fUpper,
        Complex whole,
        double tolerance,
        int depth)
    {
        const auto middle = (lower + upper) / 2.0;
        const auto leftMiddle = (lower + middle) / 2.0;
        const auto rightMiddle = (middle + upper) / 2.0;
        const auto fLeftMiddle = function(leftMiddle);
        const auto fRightMiddle = function(rightMiddle);
        const auto left =
            (middle - lower) / 6.0 * (fLower + 4.0 * fLeftMiddle + fMiddle);
        const auto right =
            (upper - middle) / 6.0 * (fMiddle + 4.0 * fRightMiddle + fUpper);
        const auto error = std::abs(left + right - whole);

        if (depth <= 0 || error <= 15.0 * tolerance)
            return left + right + (left + right - whole) / 15.0;

        return adaptiveSimpson(
                   function,
                   lower,
                   middle,
                   fLower,
                   fLeftMiddle,
                   fMiddle,
                   left,
                   tolerance / 2.0,
                   depth - 1)
            + adaptiveSimpson(
                   function,
                   middle,
                   upper,
                   fMiddle,
                   fRightMiddle,
                   fUpper,
                   right,
                   tolerance / 2.0,
                   depth - 1);
    }

    // Scaled complex complementary error function erfcx(z).
    //
    // For Re(z)>=0 an absolutely convergent real-axis representation is
    //   erfcx(z) = 2/sqrt(pi) * integral_0^inf exp(-u^2 - 2 z u) du.
    // Moderate arguments use deterministic adaptive quadrature of this exact
    // representation. Large arguments use the standard asymptotic expansion.
    // This is numerical infrastructure for the exact Green-function formula,
    // not a physical approximation or a heritable/runtime organism model.
    Complex complexErfcx(Complex z)
    {
        if (!std::isfinite(z.real()) || !std::isfinite(z.imag()) || z.real() < -1.0e-13)
            throw std::domain_error("CHEM-1B complex erfcx argument left its supported right-half-plane domain.");

        if (z.real() < 0.0) z.real(0.0);

        if (std::abs(z) > 8.0)
            return erfcxAsymptotic(z);

        if (std::abs(z.imag()) < 1.0e-15)
            return {std::exp(z.real() * z.real()) * std::erfc(z.real()), 0.0};

        const auto integrand = [z](double u)
        {
            return std::exp(Complex{-u * u, 0.0} - 2.0 * z * u);
        };

        constexpr double upperLimit = 8.0;
        const auto segments = std::max(
            8,
            static_cast<int>(std::ceil(
                2.0 * std::abs(z.imag()) * upperLimit / (std::numbers::pi / 2.0))));
        const auto width = upperLimit / static_cast<double>(segments);
        constexpr double totalTolerance = 5.0e-15;

        Complex integral{};
        for (int segment = 0; segment < segments; ++segment)
        {
            const auto lower = segment * width;
            const auto upper = (segment + 1) * width;
            const auto middle = (lower + upper) / 2.0;
            const auto fLower = integrand(lower);
            const auto fMiddle = integrand(middle);
            const auto fUpper = integrand(upper);
            const auto whole =
                (upper - lower) / 6.0 * (fLower + 4.0 * fMiddle + fUpper);
            integral += adaptiveSimpson(
                integrand,
                lower,
                upper,
                fLower,
                fMiddle,
                fUpper,
                whole,
                totalTolerance / static_cast<double>(segments),
                14);
        }
        return 2.0 / kSqrtPi * integral;
    }

    Complex scaledW(double x, Complex y)
    {
        // W(x,y)=exp(2xy+y^2)erfc(x+y)=exp(-x^2)erfcx(x+y).
        return std::exp(-x * x) * complexErfcx(Complex{x, 0.0} + y);
    }

    ae::ChemicalAssociationSpatialRoots makeRoots(
        const ae::ChemicalAssociationParameters& parameters)
    {
        const ae::ReversibleChemicalAssociation reduced(parameters);
        const auto chi = reduced.rates().intrinsicToDiffusionRatio;
        const auto kd = parameters.intrinsicDissociationRate();
        const auto delta = kd == 0.0
            ? 0.0
            : scaledProductRatio(
                  kd,
                  parameters.encounterRadius(),
                  parameters.encounterRadius(),
                  parameters.relativeDiffusionCoefficient());

        if (delta == 0.0)
        {
            return {{{Complex{0.0, 0.0}, Complex{0.0, 0.0}, Complex{1.0 + chi, 0.0}}},
                0.0,
                true};
        }

        const auto roots = solveDimensionlessRoots(chi, delta);
        const auto separation = relativeRootSeparation(roots);
        if (!std::isfinite(separation) || separation < kRootSeparationFloor)
            throw std::domain_error(
                "CHEM-1B characteristic roots are too nearly degenerate for the current double-precision A12 evaluator.");

        return {roots, separation, false};
    }

    double checkedDensity(Complex numerator, double denominator, double cancellationScale)
    {
        if (!std::isfinite(numerator.real()) || !std::isfinite(numerator.imag())
            || !std::isfinite(denominator) || denominator <= 0.0)
            throw std::overflow_error("CHEM-1B probability density left representable double range.");

        const auto residueScale = std::max(1.0, std::abs(numerator.real()));
        if (std::abs(numerator.imag()) > kImaginaryResidueRelativeTolerance * residueScale)
            throw std::runtime_error(
                "CHEM-1B conjugate-root cancellation left a material imaginary residue.");

        auto realNumerator = numerator.real();
        if (realNumerator < 0.0)
        {
            if (-realNumerator > kNegativeCancellationRelativeTolerance * std::max(1.0, cancellationScale))
                throw std::runtime_error(
                    "CHEM-1B exact-density evaluation produced a materially negative probability density.");
            realNumerator = 0.0;
        }

        const auto density = realNumerator / denominator;
        if (!std::isfinite(density) || density < 0.0)
            throw std::overflow_error("CHEM-1B probability density is not representable.");
        return density;
    }

    ae::PhysicalQuantityDescriptor quantity(
        const char* key,
        const char* unit,
        const char* description)
    {
        return {key, std::string("chem1.spatial.") + key, unit, description};
    }

    ae::PhysicalCouplingProcessSchema makeSpatialSchema()
    {
        ae::PhysicalCouplingProcessSchema schema;
        schema.identifier = "alien_evolution.chem1.spatial_reference.schema";
        schema.model.identifier = "alien_evolution.chem1.spatial_reference";
        schema.model.name = "CHEM-1 exact reversible isolated-pair spatial reference";
        schema.model.version = "chem1b-v1";
        schema.model.description =
            "M7B exact radial Green-function reference for one isolated pair in unbounded 3-D; "
            "not a reservoir, many-particle solver, production sensing path, or universal chemistry model.";
        schema.model.parameters = {
            {"a", "m", "Spherical encounter radius used as calibration geometry, not anatomy"},
            {"D", "m^2/s", "Relative diffusion coefficient"},
            {"k_a", "m^3/s", "Intrinsic Collins-Kimball association constant"},
            {"k_d", "1/s", "Intrinsic dissociation rate"}};
        schema.model.assumptions = {
            "One isolated pair in unbounded three-dimensional space",
            "Spherical encounter support with finite Collins-Kimball contact reactivity",
            "Reversible back-reaction boundary condition; no external force or interaction potential",
            "Ideal Brownian relative diffusion with constant D",
            "Exact published Green-function model; numerical root/special-function evaluation remains finite precision",
            "Irreversible k_d=0 uses the exact radiation-boundary analytical limit",
            "No maintained reservoir concentration, depletion bath, mixtures, crowding, or many-particle interference",
            "Reference model is simulator-owned and never inherited or evolvable"};
        schema.model.validityScope =
            "CHEM-1B isolated-pair unbounded-3D radial Green-function reference only; "
            "finite-domain/reservoir/many-particle claims remain unresolved.";
        schema.model.uncertainties = {
            ae::UncertaintyKind::NumericalReduction,
            ae::UncertaintyKind::ScientificModelForm};
        schema.model.evidence = {{
            ae::EvidenceStatus::EstablishedPhysicalInteraction,
            "Exact Green-function solution of the chosen 3-D reversible Smoluchowski/Collins-Kimball isolated-pair model.",
            {
                {"Kim and Shin (1999)", "https://doi.org/10.1103/PhysRevLett.82.1578",
                    "Exact reversible isolated-pair Green function"},
                {"Prustel and Meier-Schellersheim (2021)", "https://doi.org/10.1063/5.0037266",
                    "Appendix A reproduces the 3-D reversible radial propagator and root relations"}},
            "P1 supports the physical reference model, not biological sensory use or alien calibration."}};
        schema.ports = {
            {"initial_separation",
                quantity("initial_separation", "m", "Initial center-to-center pair separation r0 >= a"),
                ae::PortDirectionality::Input}};
        schema.observables = {
            quantity("volume_probability_density", "1/m^3",
                "Unbound spatial probability density at radial separation r and elapsed time t"),
            quantity("radial_shell_probability_density", "1/m",
                "Probability density with respect to radial shell coordinate r")};
        schema.regionBindingIdentifier = "chem1.ideal_spherical_pair_reference";
        schema.validate();
        return schema;
    }

    ae::AdaptivePhysicsContract makeSpatialContract(
        const ae::PhysicalCouplingProcessSchema& schema)
    {
        ae::AdaptivePhysicsContract contract{
            {schema.model.identifier, schema.model.version},
            schema.observables,
            {
                {"isolated_pair", "chem1b.unbounded_isolated_pair",
                    "Exactly one pair in unbounded 3-D; not a maintained reservoir or finite bath.", std::nullopt},
                {"spherical_contact", "chem1b.sck_backreaction_sphere",
                    "Spherical encounter support with Collins-Kimball contact reactivity and reversible back-reaction.", std::nullopt},
                {"numerical_conditioning", "chem1b.root_separation",
                    "Current A12 evaluator rejects sufficiently near-degenerate characteristic roots instead of silently amplifying cancellation.",
                    "Numerical policy only; not a scientific validity threshold"}},
            {
                {"volume_density_numerical_error", "volume_probability_density",
                    "chem1b.reference_golden_difference", "1/m^3",
                    "Difference from independent high-precision A12 fixtures; numerical error is separate from model-form uncertainty."},
                {"shell_density_numerical_error", "radial_shell_probability_density",
                    "chem1b.reference_golden_difference", "1/m",
                    "Derived from the same exact radial reference; no claim of reservoir-occupancy certification."}},
            {},
            "Exact isolated-pair spatial reference for its declared density QoIs. "
            "This is a reference role, not a scalar fidelity rank or automatic runtime-selection policy."};
        contract.validate();
        return contract;
    }
}

namespace ae
{
    ReversibleChemicalAssociationSpatialReference::ReversibleChemicalAssociationSpatialReference(
        ChemicalAssociationParameters parameters)
        : parameters_(parameters),
          roots_(makeRoots(parameters_))
    {}

    const ChemicalAssociationParameters&
        ReversibleChemicalAssociationSpatialReference::parameters() const
    {
        return parameters_;
    }

    ChemicalAssociationSpatialCoordinates
        ReversibleChemicalAssociationSpatialReference::dimensionlessCoordinates(
            double radius,
            double initialRadius,
            double elapsedTime) const
    {
        positive(radius, "CHEM-1B radius must be finite and positive.");
        positive(initialRadius, "CHEM-1B initial radius must be finite and positive.");
        positive(elapsedTime, "CHEM-1B elapsed time must be finite and positive.");

        const auto a = parameters_.encounterRadius();
        if (radius < a || initialRadius < a)
            throw std::invalid_argument("CHEM-1B pair separation must satisfy r >= a and r0 >= a.");

        const ReversibleChemicalAssociation reduced(parameters_);
        const auto chi = reduced.rates().intrinsicToDiffusionRatio;
        const auto kd = parameters_.intrinsicDissociationRate();
        const auto delta = kd == 0.0
            ? 0.0
            : scaledProductRatio(
                  kd,
                  a,
                  a,
                  parameters_.relativeDiffusionCoefficient());
        const auto rho = radius / a;
        const auto rho0 = initialRadius / a;
        const auto tau =
            parameters_.relativeDiffusionCoefficient() * elapsedTime / (a * a);

        if (!std::isfinite(rho) || !std::isfinite(rho0) || !std::isfinite(tau) || tau <= 0.0)
            throw std::overflow_error("CHEM-1B dimensionless coordinates are outside representable double range.");

        return {chi, delta, rho, rho0, tau};
    }

    const ChemicalAssociationSpatialRoots&
        ReversibleChemicalAssociationSpatialReference::characteristicRoots() const
    {
        return roots_;
    }

    double ReversibleChemicalAssociationSpatialReference::volumeProbabilityDensity(
        double radius,
        double initialRadius,
        double elapsedTime) const
    {
        (void)dimensionlessCoordinates(radius, initialRadius, elapsedTime);

        const auto a = parameters_.encounterRadius();
        const auto D = parameters_.relativeDiffusionCoefficient();
        const auto ka = parameters_.intrinsicAssociationConstant();
        const auto kd = parameters_.intrinsicDissociationRate();
        const auto sqrtDt = std::sqrt(D * elapsedTime);
        if (!std::isfinite(sqrtDt) || sqrtDt <= 0.0)
            throw std::overflow_error("CHEM-1B diffusion length is outside representable double range.");

        const auto difference = radius - initialRadius;
        const auto imageDifference = radius + initialRadius - 2.0 * a;
        const auto firstGaussian =
            std::exp(-(difference * difference) / (4.0 * D * elapsedTime));
        const auto secondGaussian =
            std::exp(-(imageDifference * imageDifference) / (4.0 * D * elapsedTime));

        if (kd == 0.0)
        {
            const ReversibleChemicalAssociation reduced(parameters_);
            const auto chi = reduced.rates().intrinsicToDiffusionRatio;
            const auto kappa = (1.0 + chi) / a;
            const auto x = imageDifference / (2.0 * sqrtDt);
            const auto y = kappa * sqrtDt;
            const auto w = scaledW(x, {y, 0.0}).real();
            const auto reactionTerm =
                kappa * std::sqrt(4.0 * std::numbers::pi * D * elapsedTime) * w;
            const auto numerator =
                firstGaussian + secondGaussian - reactionTerm;
            const auto cancellationScale =
                std::abs(firstGaussian) + std::abs(secondGaussian) + std::abs(reactionTerm);
            const auto denominator =
                8.0 * std::numbers::pi * radius * initialRadius
                * std::sqrt(std::numbers::pi * D * elapsedTime);
            return checkedDensity({numerator, 0.0}, denominator, cancellationScale);
        }

        if (roots_.relativeSeparation < kRootSeparationFloor)
            throw std::domain_error(
                "CHEM-1B roots are too nearly degenerate for the current exact-density evaluator.");

        const auto rootScale = std::sqrt(D) / a;
        const auto alpha = roots_.dimensionlessRoots[0] * rootScale;
        const auto beta = roots_.dimensionlessRoots[1] * rootScale;
        const auto gamma = roots_.dimensionlessRoots[2] * rootScale;

        const auto x = imageDifference / (2.0 * sqrtDt);
        const auto coefficientAlpha =
            alpha * (gamma + alpha) * (alpha + beta)
            / ((gamma - alpha) * (alpha - beta));
        const auto coefficientBeta =
            beta * (alpha + beta) * (beta + gamma)
            / ((alpha - beta) * (beta - gamma));
        const auto coefficientGamma =
            gamma * (beta + gamma) * (gamma + alpha)
            / ((beta - gamma) * (gamma - alpha));

        const auto base =
            Complex{firstGaussian + secondGaussian, 0.0}
            / std::sqrt(4.0 * std::numbers::pi * elapsedTime);
        const auto alphaTerm =
            coefficientAlpha * scaledW(x, alpha * std::sqrt(elapsedTime));
        const auto betaTerm =
            coefficientBeta * scaledW(x, beta * std::sqrt(elapsedTime));
        const auto gammaTerm =
            coefficientGamma * scaledW(x, gamma * std::sqrt(elapsedTime));
        const auto numerator = base + alphaTerm + betaTerm + gammaTerm;
        const auto cancellationScale =
            std::abs(base) + std::abs(alphaTerm) + std::abs(betaTerm) + std::abs(gammaTerm);

        // Divide sequentially to avoid avoidable radius-product overflow.
        const auto denominator =
            4.0 * std::numbers::pi * radius * initialRadius * std::sqrt(D);
        return checkedDensity(numerator, denominator, cancellationScale);
    }

    double ReversibleChemicalAssociationSpatialReference::radialShellProbabilityDensity(
        double radius,
        double initialRadius,
        double elapsedTime) const
    {
        const auto density =
            volumeProbabilityDensity(radius, initialRadius, elapsedTime);
        const auto shell =
            4.0 * std::numbers::pi * radius * radius * density;
        if (!std::isfinite(shell) || shell < 0.0)
            throw std::overflow_error("CHEM-1B radial-shell density is not representable.");
        return shell;
    }

    const PhysicalCouplingProcessSchema&
        ReversibleChemicalAssociationSpatialReference::schema()
    {
        static const auto value = makeSpatialSchema();
        return value;
    }

    const ScientificModelMetadata&
        ReversibleChemicalAssociationSpatialReference::scientificMetadata()
    {
        return schema().model;
    }

    const AdaptivePhysicsContract&
        ReversibleChemicalAssociationSpatialReference::adaptivePhysicsContract()
    {
        static const auto value = makeSpatialContract(schema());
        return value;
    }
} // namespace ae
