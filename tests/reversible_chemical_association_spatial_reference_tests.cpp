#include "alien_evolution/physics/ReversibleChemicalAssociationSpatialReference.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    using Reference = ae::ReversibleChemicalAssociationSpatialReference;

    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    void nearRelative(double actual, double expected, double tolerance = 2.0e-10)
    {
        require(std::isfinite(actual) && std::isfinite(expected), "Nonfinite comparison");
        const auto scale = std::max({1.0e-300, std::abs(actual), std::abs(expected)});
        require(std::abs(actual - expected) <= tolerance * scale, "Relative numerical comparison failed");
    }

    template <class Exception = std::invalid_argument, class Function>
    void rejects(Function function)
    {
        try
        {
            function();
        }
        catch (const Exception&)
        {
            return;
        }
        throw std::runtime_error("Expected exception was not thrown");
    }

    ae::ChemicalAssociationParameters parameters(double chi, double delta)
    {
        constexpr double a = 1.0;
        constexpr double D = 1.0;
        return {
            a,
            D,
            4.0 * std::numbers::pi * D * a * chi,
            delta * D / (a * a)};
    }

    void dimensionlessAndRoots()
    {
        const Reference reference(parameters(1.0, 1.0));
        const auto coordinates = reference.dimensionlessCoordinates(1.5, 1.1, 2.0);
        nearRelative(coordinates.chi, 1.0, 5.0e-14);
        nearRelative(coordinates.delta, 1.0, 5.0e-14);
        nearRelative(coordinates.rho, 1.5, 5.0e-14);
        nearRelative(coordinates.rho0, 1.1, 5.0e-14);
        nearRelative(coordinates.tau, 2.0, 5.0e-14);

        const auto& roots = reference.characteristicRoots();
        require(!roots.usesIrreversibleLimit, "Reversible case incorrectly used irreversible root limit");
        require(roots.relativeSeparation > 0.0 && std::isfinite(roots.relativeSeparation),
            "Root separation diagnostic invalid");

        bool foundComplex = false;
        for (const auto& root : roots.dimensionlessRoots)
        {
            const auto residual =
                root * root * root
                - 2.0 * root * root
                + root
                - 1.0;
            require(std::abs(residual) < 2.0e-12, "Characteristic root does not satisfy CHEM-1B cubic");
            foundComplex = foundComplex || std::abs(root.imag()) > 1.0e-8;
        }
        require(foundComplex, "Complex-conjugate CHEM-1B root regime was not exercised");

        // Exercise a distinct physical regime with three real characteristic roots.
        const Reference threeReal(parameters(20.0, 100.0));
        for (const auto& root : threeReal.characteristicRoots().dimensionlessRoots)
            require(std::abs(root.imag()) < 2.0e-10, "Three-real-root CHEM-1B regime was not preserved");

        const auto sum =
            roots.dimensionlessRoots[0]
            + roots.dimensionlessRoots[1]
            + roots.dimensionlessRoots[2];
        const auto pairSum =
            roots.dimensionlessRoots[0] * roots.dimensionlessRoots[1]
            + roots.dimensionlessRoots[1] * roots.dimensionlessRoots[2]
            + roots.dimensionlessRoots[2] * roots.dimensionlessRoots[0];
        const auto product =
            roots.dimensionlessRoots[0]
            * roots.dimensionlessRoots[1]
            * roots.dimensionlessRoots[2];
        require(std::abs(sum - std::complex<double>{2.0, 0.0}) < 2.0e-12,
            "Root Vieta sum mismatch");
        require(std::abs(pairSum - std::complex<double>{1.0, 0.0}) < 2.0e-12,
            "Root Vieta pair-sum mismatch");
        require(std::abs(product - std::complex<double>{1.0, 0.0}) < 2.0e-12,
            "Root Vieta product mismatch");

        rejects([&] { (void)reference.dimensionlessCoordinates(0.9, 1.1, 1.0); });
        rejects([&] { (void)reference.dimensionlessCoordinates(1.1, 0.9, 1.0); });
        rejects([&] { (void)reference.dimensionlessCoordinates(1.1, 1.1, 0.0); });
        rejects([&] { (void)reference.dimensionlessCoordinates(
            std::numeric_limits<double>::quiet_NaN(), 1.1, 1.0); });

        rejects<std::domain_error>([] {
            (void)Reference(parameters(1.0, 1.0e-30));
        });
    }

    struct GoldenCase
    {
        double chi;
        double delta;
        double rho0;
        double rho;
        double tau;
        double density;
    };

    void reversibleGoldenValues()
    {
        // Independently generated with mpmath 1.3.0 at 80 decimal digits from
        // Prüstel & Meier-Schellersheim (2021), Appendix A, Eqs. A12-A13.
        // a=D=1, k_a=4*pi*chi, k_d=delta.
        const std::array<GoldenCase, 7> cases{{
            {0.01, 0.1, 1.01, 1.2, 0.01,
                0.1280371057457073416411382474452704113},
            {1.0, 1.0, 1.1, 1.5, 1.0,
                0.008112199825147342402393627942132403887},
            {100.0, 0.1, 2.0, 2.5, 0.1,
                0.007599430454720642064126827209514821946},
            {10.0, 100.0, 1.0, 1.0, 0.001,
                0.8514528460871969288359479948792505183},
            {0.1, 10.0, 3.0, 4.0, 10.0,
                0.0004018804150016235649482875518394756220},
            {1.0, 1.0, 2.0, 3.0, 2.0,
                0.002112269419136343261181113684968273559},
            {20.0, 100.0, 1.2, 2.0, 0.2,
                0.009627860563879113272084888806353082067}
        }};

        for (const auto& fixture : cases)
        {
            const Reference reference(parameters(fixture.chi, fixture.delta));
            const auto actual =
                reference.volumeProbabilityDensity(
                    fixture.rho,
                    fixture.rho0,
                    fixture.tau);
            nearRelative(actual, fixture.density);

            const auto shell =
                reference.radialShellProbabilityDensity(
                    fixture.rho,
                    fixture.rho0,
                    fixture.tau);
            nearRelative(
                shell,
                4.0 * std::numbers::pi
                    * fixture.rho * fixture.rho * fixture.density);
        }
    }

    void irreversibleLimitGoldenValues()
    {
        // Independent mpmath 1.3.0 / 80-digit A3 radiation-boundary fixtures for k_d=0.
        const std::array<GoldenCase, 3> cases{{
            {0.01, 0.0, 1.1, 1.5, 0.1,
                0.03912061931382769905933833474951107143},
            {1.0, 0.0, 1.1, 1.5, 1.0,
                0.005517205205500838641480614849018085893},
            {100.0, 0.0, 2.0, 2.5, 0.1,
                0.007599430454442569582408676994167809194}
        }};

        for (const auto& fixture : cases)
        {
            const Reference reference(parameters(fixture.chi, fixture.delta));
            require(reference.characteristicRoots().usesIrreversibleLimit,
                "k_d=0 did not use exact irreversible analytical limit");
            nearRelative(
                reference.volumeProbabilityDensity(
                    fixture.rho,
                    fixture.rho0,
                    fixture.tau),
                fixture.density);
        }
    }

    void densityBehavior()
    {
        const Reference reference(parameters(1.0, 1.0));
        for (const auto radius : {1.0, 1.01, 1.1, 1.5, 2.0, 5.0, 10.0})
        {
            const auto density =
                reference.volumeProbabilityDensity(radius, 1.1, 0.5);
            require(std::isfinite(density) && density >= 0.0,
                "Exact spatial reference density left physical range");
        }

        const auto nearDensity =
            reference.volumeProbabilityDensity(1.5, 1.1, 1.0);
        const auto farDensity =
            reference.volumeProbabilityDensity(100.0, 1.1, 1.0);
        require(farDensity < nearDensity, "Spatial density did not decay at large separation");

        require(
            reference.volumeProbabilityDensity(1.5, 1.1, 1.0)
                == reference.volumeProbabilityDensity(1.5, 1.1, 1.0),
            "Exact reference evaluation is not deterministic");
    }

    void metadataAndScope()
    {
        const auto& metadata = Reference::scientificMetadata();
        const auto& schema = Reference::schema();
        const auto& contract = Reference::adaptivePhysicsContract();

        metadata.validate();
        schema.validate();
        contract.validate();

        require(metadata.identifier == "alien_evolution.chem1.spatial_reference"
            && metadata.version == "chem1b-v1",
            "M7B reference identity changed");
        require(metadata.description
            && metadata.description->find("isolated pair") != std::string::npos,
            "M7B isolated-pair scope missing");
        require(metadata.validityScope.find("many-particle") != std::string::npos,
            "M7B many-particle limitation missing");
        require(std::none_of(
            metadata.assumptions.begin(),
            metadata.assumptions.end(),
            [](const std::string& value)
            {
                return value.find("Langmuir") != std::string::npos;
            }),
            "M7B silently became a Langmuir reference");
        require(std::any_of(
            metadata.assumptions.begin(),
            metadata.assumptions.end(),
            [](const std::string& value)
            {
                return value.find("No maintained reservoir concentration") != std::string::npos;
            }),
            "M7B isolated-pair/no-reservoir guardrail missing");
        require(metadata.evidence.size() == 1
            && metadata.evidence[0].status == ae::EvidenceStatus::EstablishedPhysicalInteraction
            && metadata.evidence[0].references.size() == 2,
            "M7B evidence/provenance record changed");

        require(schema.state.empty() && schema.history.empty() && schema.control.empty(),
            "Reference oracle became a runtime PCP implementation");
        require(schema.regionBindingIdentifier
            && *schema.regionBindingIdentifier == "chem1.ideal_spherical_pair_reference",
            "M7B calibration-geometry identifier changed");
        require(schema.observables.size() == 2
            && schema.observables[0].key == "volume_probability_density"
            && schema.observables[1].key == "radial_shell_probability_density",
            "M7B QoI scope expanded silently");

        require(contract.model
            == ae::ScientificModelRef("alien_evolution.chem1.spatial_reference", "chem1b-v1"),
            "M7B ACP identity mismatch");
        require(contract.referenceModels.empty(), "Reference oracle acquired a fake higher-fidelity edge");
        require(contract.description
            && contract.description->find("not a scalar fidelity rank") != std::string::npos,
            "M7B ACP multidimensional-fidelity guardrail lost");
        require(std::any_of(
            contract.validityCriteria.begin(),
            contract.validityCriteria.end(),
            [](const auto& criterion)
            {
                return criterion.key == "numerical_conditioning";
            }),
            "M7B numerical-conditioning declaration missing");
    }
}

int main()
{
    try
    {
        dimensionlessAndRoots();
        reversibleGoldenValues();
        irreversibleLimitGoldenValues();
        densityBehavior();
        metadataAndScope();
        std::cout << "CHEM-1B exact isolated-pair spatial-reference tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
