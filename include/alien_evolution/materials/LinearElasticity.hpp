#pragma once

#include <array>

namespace ae
{
    class Rotation3;

    struct SymmetricTensor3
    {
        double xx = 0.0;
        double yy = 0.0;
        double zz = 0.0;
        double yz = 0.0;
        double xz = 0.0;
        double xy = 0.0;

        [[nodiscard]] static SymmetricTensor3 identity(double scale = 1.0);
        [[nodiscard]] static SymmetricTensor3 fromKelvin(
            const std::array<double, 6>& values);

        [[nodiscard]] std::array<double, 6> kelvin() const;
        [[nodiscard]] double trace() const;
        [[nodiscard]] SymmetricTensor3 deviator() const;
        [[nodiscard]] double inner(const SymmetricTensor3& other) const;
        [[nodiscard]] double norm() const;
        [[nodiscard]] SymmetricTensor3 rotated(const Rotation3& rotation) const;

        [[nodiscard]] SymmetricTensor3 operator+(const SymmetricTensor3& other) const;
        [[nodiscard]] SymmetricTensor3 operator-(const SymmetricTensor3& other) const;
        [[nodiscard]] SymmetricTensor3 operator*(double scalar) const;
    };

    class Rotation3
    {
    public:
        explicit Rotation3(std::array<double, 9> rowMajor);

        [[nodiscard]] static Rotation3 identity();
        [[nodiscard]] static Rotation3 fromAxisAngle(
            std::array<double, 3> axis,
            double radians);
        // Returns a proper local->global rotation whose third local basis
        // vector is the requested layer normal.
        [[nodiscard]] static Rotation3 fromLayerNormal(
            std::array<double, 3> layerNormal);

        [[nodiscard]] double at(std::size_t row, std::size_t column) const;
        [[nodiscard]] const std::array<double, 9>& matrix() const;
        [[nodiscard]] Rotation3 transposed() const;
        void validate() const;

    private:
        std::array<double, 9> matrix_;
    };

    class KelvinElasticityTensor
    {
    public:
        explicit KelvinElasticityTensor(std::array<double, 36> rowMajor);

        [[nodiscard]] static KelvinElasticityTensor isotropic(
            double bulkModulus,
            double shearModulus);

        [[nodiscard]] double at(std::size_t row, std::size_t column) const;
        [[nodiscard]] const std::array<double, 36>& matrix() const;
        [[nodiscard]] SymmetricTensor3 stress(const SymmetricTensor3& strain) const;
        [[nodiscard]] double energyDensity(const SymmetricTensor3& strain) const;
        [[nodiscard]] KelvinElasticityTensor rotated(const Rotation3& rotation) const;
        [[nodiscard]] KelvinElasticityTensor operator+(
            const KelvinElasticityTensor& other) const;
        [[nodiscard]] KelvinElasticityTensor operator*(double scalar) const;

        [[nodiscard]] bool approximatelyEqual(
            const KelvinElasticityTensor& other,
            double relativeTolerance = 1.0e-11) const;

        void validate() const;

    private:
        std::array<double, 36> matrix_;
    };

    struct IsotropicLinearElasticity
    {
        double bulkModulus;
        double shearModulus;

        void validate() const;

        [[nodiscard]] double lameLambda() const;
        [[nodiscard]] double youngModulus() const;
        [[nodiscard]] double poissonRatio() const;
        [[nodiscard]] KelvinElasticityTensor tensor() const;
    };
} // namespace ae
