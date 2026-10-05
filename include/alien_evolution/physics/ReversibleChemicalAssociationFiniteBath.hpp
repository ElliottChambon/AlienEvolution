#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "alien_evolution/physics/ReversibleChemicalAssociation.hpp"

namespace ae
{
    struct FiniteChemicalBathConfig
    {
        double outerRadius = 2.0;
        double spectralTolerance = 1.0e-12;
        double minimumResolvedDimensionlessTime = 1.0e-5;
        std::size_t maxModes = 32768;
    };

    struct FiniteChemicalBathState
    {
        // One radius per conserved ligand. If boundLigand is set, that ligand is
        // stored exactly at encounter radius a while bound.
        std::vector<double> ligandRadii;
        std::optional<std::size_t> boundLigand;
    };

    enum class FiniteChemicalBathEventKind
    {
        Binding,
        Dissociation
    };

    struct FiniteChemicalBathEvent
    {
        double time;
        FiniteChemicalBathEventKind kind;
        std::size_t ligandIndex;
    };

    struct FiniteChemicalBathTrajectory
    {
        FiniteChemicalBathState initialState;
        FiniteChemicalBathState finalState;
        double duration;
        std::vector<FiniteChemicalBathEvent> events;
    };

    struct FiniteChemicalBathDiagnostics
    {
        double chi;
        double delta;
        double lambda;
        double accessibleVolume;
        double minimumResolvedPhysicalTime;
        std::size_t reactiveModeCount;
        std::size_t reflectingModeCount;
    };

    // M7C calibration/reference model: N identical ligands compete for one
    // capacity-one spherical site inside a concentric reflecting spherical bath.
    // Spherical symmetry reduces every unbound ligand to a radial coordinate.
    //
    // This is high-fidelity reference infrastructure for the declared CHEM-1
    // benchmark only. It is not a general 3-D particle engine, production sensing
    // path, universal chemistry model, or heritable solver.
    class ReversibleChemicalAssociationFiniteBath
    {
    public:
        ReversibleChemicalAssociationFiniteBath(
            ChemicalAssociationParameters parameters,
            FiniteChemicalBathConfig config);

        [[nodiscard]] const ChemicalAssociationParameters& parameters() const;
        [[nodiscard]] const FiniteChemicalBathConfig& config() const;
        [[nodiscard]] const FiniteChemicalBathDiagnostics& diagnostics() const;

        [[nodiscard]] double accessibleVolume() const;
        [[nodiscard]] double equilibriumBoundProbability(std::size_t totalLigands) const;
        [[nodiscard]] double meanFirstReactionTime(double initialRadius) const;

        [[nodiscard]] double reactiveSurvival(
            double initialRadius,
            double elapsedTime) const;
        [[nodiscard]] double reactiveFirstReactionDensity(
            double initialRadius,
            double elapsedTime) const;
        [[nodiscard]] double reactiveRadialShellProbabilityDensity(
            double radius,
            double initialRadius,
            double elapsedTime) const;
        [[nodiscard]] double reactiveConditionalRadialCdf(
            double radius,
            double initialRadius,
            double elapsedTime) const;

        [[nodiscard]] double reflectingRadialShellProbabilityDensity(
            double radius,
            double initialRadius,
            double elapsedTime) const;
        [[nodiscard]] double reflectingRadialCdf(
            double radius,
            double initialRadius,
            double elapsedTime) const;

        void validateState(const FiniteChemicalBathState& state) const;

        [[nodiscard]] FiniteChemicalBathTrajectory sampleTrajectory(
            const FiniteChemicalBathState& initialState,
            double duration,
            Random& random,
            std::size_t eventLimit = 100000) const;

        [[nodiscard]] static const ScientificModelMetadata& scientificMetadata();
        [[nodiscard]] static const PhysicalCouplingProcessSchema& schema();
        [[nodiscard]] static const AdaptivePhysicsContract& adaptivePhysicsContract();

    private:
        struct SpectralMode
        {
            double z;
            double norm;
            double fullRadialIntegral;
        };

        ChemicalAssociationParameters parameters_;
        FiniteChemicalBathConfig config_;
        FiniteChemicalBathDiagnostics diagnostics_;
        std::vector<SpectralMode> reactiveModes_;
        std::vector<SpectralMode> reflectingModes_;

        [[nodiscard]] double dimensionlessTime(double elapsedTime) const;
        [[nodiscard]] double reactiveSurvivalDimensionless(
            double rho0,
            double tau) const;
        [[nodiscard]] double reactiveFirstReactionDensityDimensionless(
            double rho0,
            double tau) const;
        [[nodiscard]] double reactiveConditionalCdfDimensionless(
            double rho,
            double rho0,
            double tau) const;
        [[nodiscard]] double reflectingCdfDimensionless(
            double rho,
            double rho0,
            double tau) const;

        [[nodiscard]] double sampleReactiveConditionalRadius(
            double initialRadius,
            double elapsedTime,
            Random& random) const;
        [[nodiscard]] double sampleReflectingRadius(
            double initialRadius,
            double elapsedTime,
            Random& random) const;
    };
} // namespace ae
