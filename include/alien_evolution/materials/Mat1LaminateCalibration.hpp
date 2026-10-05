#pragma once

#include <array>
#include <optional>
#include <string>

#include "alien_evolution/materials/MaterialCompiler.hpp"

namespace ae
{
    struct Mat1BenchmarkPhase
    {
        std::string identifier;
        double axialElasticModulus;

        void validate() const;
    };

    struct Mat1LaminateMaterialState
    {
        DevelopedMaterialStateEnvelope envelope;
        Mat1BenchmarkPhase phaseA;
        Mat1BenchmarkPhase phaseB;
        double phaseAFraction;
        double laminatePeriod;
        std::array<double, 3> layerNormal;

        Mat1LaminateMaterialState(
            std::string stateIdentifier,
            Mat1BenchmarkPhase phaseA,
            Mat1BenchmarkPhase phaseB,
            double phaseAFraction,
            double laminatePeriod,
            std::array<double, 3> layerNormal);

        [[nodiscard]] double phaseBFraction() const;
        void validate() const;
    };

    struct Mat1AxialResponseQuery
    {
        std::array<double, 3> loadingDirection;
        std::optional<double> requestedSpatialScale;

        Mat1AxialResponseQuery(
            std::array<double, 3> loadingDirection,
            std::optional<double> requestedSpatialScale = std::nullopt);

        void validate() const;
    };

    enum class Mat1AxialCalibrationMode
    {
        InPlaneIsoStrain,
        LayerNormalIsoStress
    };

    struct Mat1AxialCompilationResult
    {
        MaterialCompilationRecord record;
        std::optional<double> effectiveAxialModulus;
        std::optional<Mat1AxialCalibrationMode> calibrationMode;
        std::optional<double> requestedScaleToPeriodRatio;

        void validate() const;
    };

    // MAT-1 is a deliberately scalar calibration of the Material Compiler
    // architecture. It is exact only for the stated one-dimensional
    // iso-strain / iso-stress laminate boundary-value problems and must not be
    // interpreted as a general 3-D composite Young's-modulus law.
    class Mat1LaminateCalibrationCompiler
    {
    public:
        [[nodiscard]] static Mat1AxialCompilationResult compile(
            const Mat1LaminateMaterialState& state,
            const Mat1AxialResponseQuery& query);

        [[nodiscard]] static const ScientificModelMetadata& scientificMetadata();
    };
} // namespace ae
