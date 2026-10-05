#pragma once

#include <array>
#include <complex>

#include "alien_evolution/physics/ReversibleChemicalAssociation.hpp"

namespace ae
{
    struct ChemicalAssociationSpatialCoordinates
    {
        double chi;
        double delta;
        double rho;
        double rho0;
        double tau;
    };

    struct ChemicalAssociationSpatialRoots
    {
        std::array<std::complex<double>, 3> dimensionlessRoots;
        double relativeSeparation;
        bool usesIrreversibleLimit;
    };

    // M7B reference oracle for one isolated reversible pair in unbounded 3-D.
    //
    // This evaluates the exact radial Green's function of the approved
    // Smoluchowski/Collins-Kimball contact-reactivity model with the reversible
    // back-reaction boundary condition. It is not a many-particle solver,
    // reservoir model, production sensing path, or universal chemistry model.
    //
    // The reference is intentionally separate from the M7A well-mixed reductions:
    // an isolated pair does not approach Langmuir equilibrium in infinite 3-D.
    class ReversibleChemicalAssociationSpatialReference
    {
    public:
        explicit ReversibleChemicalAssociationSpatialReference(
            ChemicalAssociationParameters parameters);

        [[nodiscard]] const ChemicalAssociationParameters& parameters() const;
        [[nodiscard]] ChemicalAssociationSpatialCoordinates dimensionlessCoordinates(
            double radius,
            double initialRadius,
            double elapsedTime) const;
        [[nodiscard]] const ChemicalAssociationSpatialRoots& characteristicRoots() const;

        // Probability density per unit 3-D volume [1/m^3] at radial separation r.
        // Valid for r >= a, r0 >= a, t > 0.
        [[nodiscard]] double volumeProbabilityDensity(
            double radius,
            double initialRadius,
            double elapsedTime) const;

        // Radial-shell density [1/m]: probability in [r, r+dr] is approximately
        // radialShellProbabilityDensity(r,...)*dr.
        [[nodiscard]] double radialShellProbabilityDensity(
            double radius,
            double initialRadius,
            double elapsedTime) const;

        [[nodiscard]] static const ScientificModelMetadata& scientificMetadata();
        [[nodiscard]] static const PhysicalCouplingProcessSchema& schema();
        [[nodiscard]] static const AdaptivePhysicsContract& adaptivePhysicsContract();

    private:
        ChemicalAssociationParameters parameters_;
        ChemicalAssociationSpatialRoots roots_;
    };
} // namespace ae
