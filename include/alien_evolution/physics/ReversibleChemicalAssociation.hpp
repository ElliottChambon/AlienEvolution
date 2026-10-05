#pragma once

#include <cstddef>
#include <vector>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/physics/AdaptiveCertifiedPhysics.hpp"

namespace ae
{
    // CHEM-1 inputs in consistent number-based units: a [m], D [m^2/s],
    // k_a [m^3/s], k_d [1/s], ideal concentration c [1/m^3]. No units engine.
    // Sphere is a benchmark geometry, not anatomy; no heritable mutation path.
    class ChemicalAssociationParameters
    {
    public:
        ChemicalAssociationParameters(double encounterRadius, double relativeDiffusionCoefficient,
            double intrinsicAssociationConstant, double intrinsicDissociationRate);
        [[nodiscard]] double encounterRadius() const;
        [[nodiscard]] double relativeDiffusionCoefficient() const;
        [[nodiscard]] double intrinsicAssociationConstant() const;
        [[nodiscard]] double intrinsicDissociationRate() const;
    private:
        double radius_;
        double diffusion_;
        double association_;
        double dissociation_;
    };

    struct ChemicalAssociationRates
    {
        double diffusionLimitedAssociationRate; // k_D
        double intrinsicToDiffusionRatio;       // chi, not a fidelity level
        double effectiveAssociationRate;        // k_on
        double rebindingProbability;            // compiled ideal-sphere probability
        double effectiveDissociationRate;        // k_off
        double dissociationConcentration;        // K_D
    };

    // Capability choices, not an ordered fidelity ladder or runtime policy.
    enum class ChemicalAssociationRepresentation
    {
        AnalyticalRelations,
        EquilibriumReduction,
        DeterministicWellMixed,
        StochasticWellMixed
    };

    struct ChemicalAssociationTransition
    {
        double time;
        bool bound;
    };

    struct ChemicalAssociationTrajectory
    {
        bool initiallyBound;
        bool finallyBound;
        double duration;
        std::vector<ChemicalAssociationTransition> events;
    };

    // Benchmark-only L + S <-> LS: approved analytical ideal 3-D spherical
    // relations plus open-reservoir, constant-concentration well-mixed reductions.
    // Not the spatial stochastic reference, a chemical-sensing engine, or a
    // universal PCP execution interface. No DiffusiveField2D dependency.
    class ReversibleChemicalAssociation
    {
    public:
        explicit ReversibleChemicalAssociation(ChemicalAssociationParameters parameters);
        [[nodiscard]] const ChemicalAssociationParameters& parameters() const;
        [[nodiscard]] const ChemicalAssociationRates& rates() const;
        [[nodiscard]] double equilibriumOccupancy(double idealConcentration) const;
        [[nodiscard]] double relaxationTime(double idealConcentration) const;
        [[nodiscard]] double occupancyDerivative(double occupancy, double idealConcentration) const;
        [[nodiscard]] double occupancyAfter(double initialOccupancy, double idealConcentration, double elapsedTime) const;
        // Constant-concentration two-state CTMC with only the caller's ae::Random.
        // Each proposed wait consumes one nonzero uniform draw (retry zero).
        // The last wait is censored at duration but still consumes its draw.
        // Absorbing states and zero duration consume no draws. Limit is a safety
        // guard: throw on excess events, never silently truncate trajectories.
        [[nodiscard]] ChemicalAssociationTrajectory sampleTrajectory(bool initiallyBound, double idealConcentration,
            double duration, Random& random, std::size_t eventLimit = 1000000) const;

        [[nodiscard]] static const PhysicalCouplingProcessSchema& schema(ChemicalAssociationRepresentation representation);
        [[nodiscard]] static const ScientificModelMetadata& scientificMetadata(ChemicalAssociationRepresentation representation);
        [[nodiscard]] static const AdaptivePhysicsContract& adaptivePhysicsContract(ChemicalAssociationRepresentation representation);
        [[nodiscard]] static PhysicsChallengeRegistry spatialReferenceChallenges();
    private:
        ChemicalAssociationParameters parameters_;
        ChemicalAssociationRates rates_;
        [[nodiscard]] double associationHazard(double idealConcentration) const;
    };
} // namespace ae
