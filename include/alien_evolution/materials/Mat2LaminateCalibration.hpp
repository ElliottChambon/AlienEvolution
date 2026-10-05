#pragma once

#include <array>
#include <optional>
#include <string>

#include "alien_evolution/materials/LinearElasticity.hpp"
#include "alien_evolution/materials/MaterialCompiler.hpp"

namespace ae
{
    struct Mat2BenchmarkPhase
    {
        std::string identifier;
        IsotropicLinearElasticity elasticity;

        void validate() const;
    };

    struct Mat2LaminateMaterialState
    {
        DevelopedMaterialStateEnvelope envelope;
        Mat2BenchmarkPhase phaseA;
        Mat2BenchmarkPhase phaseB;
        double phaseAFraction;
        double laminatePeriod;
        std::array<double, 3> layerNormal;

        Mat2LaminateMaterialState(
            std::string stateIdentifier,
            Mat2BenchmarkPhase phaseA,
            Mat2BenchmarkPhase phaseB,
            double phaseAFraction,
            double laminatePeriod,
            std::array<double, 3> layerNormal);

        [[nodiscard]] double phaseBFraction() const;
        void validate() const;
    };

    struct Mat2ElasticResponseQuery
    {
        std::optional<double> requestedSpatialScale;

        void validate() const;
    };

    struct Mat2ElasticCompilationResult
    {
        MaterialCompilationRecord record;
        KelvinElasticityTensor effectiveTensor;
        KelvinElasticityTensor voigtUpperBound;
        KelvinElasticityTensor reussLowerBound;
        std::optional<double> requestedScaleToPeriodRatio;

        void validate() const;
    };

    struct Mat2PhaseField
    {
        SymmetricTensor3 strain;
        SymmetricTensor3 stress;
        double elasticEnergyDensity;

        void validate() const;
    };

    struct Mat2LaminateMicroscaleResponse
    {
        SymmetricTensor3 macroStrain;
        SymmetricTensor3 macroStress;
        Mat2PhaseField phaseA;
        Mat2PhaseField phaseB;
        SymmetricTensor3 volumeAverageMicroStrain;
        SymmetricTensor3 volumeAverageMicroStress;
        double macroElasticEnergyDensity;
        double volumeAverageMicroElasticEnergyDensity;
        double hillMandelWorkResidual;
        double energyResidual;

        void validate(double phaseAFraction) const;
    };

    // Exact periodic-laminate calibration for quasi-static, isothermal,
    // geometrically linear, energy-based elasticity. This is constitutive /
    // homogenization reference physics, not an organism structural solver.
    class Mat2LaminateCalibrationCompiler
    {
    public:
        [[nodiscard]] static Mat2ElasticCompilationResult compile(
            const Mat2LaminateMaterialState& state,
            const Mat2ElasticResponseQuery& query = {});

        [[nodiscard]] static Mat2LaminateMicroscaleResponse evaluateMicroscale(
            const Mat2LaminateMaterialState& state,
            const SymmetricTensor3& macroStrain);

        [[nodiscard]] static const ScientificModelMetadata& scientificMetadata();
    };
} // namespace ae
