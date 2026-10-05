#include "alien_evolution/materials/LinearElasticity.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace
{
    constexpr double kRotationTolerance = 2.0e-12;
    constexpr double kSymmetryTolerance = 2.0e-12;

    std::array<double, 3> normalized(std::array<double, 3> value)
    {
        for (const auto component : value)
            if (!std::isfinite(component))
                throw std::invalid_argument("Rotation/vector components must be finite.");

        const auto norm = std::hypot(value[0], value[1], value[2]);
        if (!std::isfinite(norm) || norm <= 0.0)
            throw std::invalid_argument("Rotation/vector norm must be finite and positive.");
        for (auto& component : value) component /= norm;
        return value;
    }

    std::array<double, 3> cross(
        const std::array<double, 3>& a,
        const std::array<double, 3>& b)
    {
        return {
            a[1] * b[2] - a[2] * b[1],
            a[2] * b[0] - a[0] * b[2],
            a[0] * b[1] - a[1] * b[0]};
    }

    double dot(
        const std::array<double, 3>& a,
        const std::array<double, 3>& b)
    {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }

    std::array<double, 9> fullMatrix(const ae::SymmetricTensor3& value)
    {
        return {
            value.xx, value.xy, value.xz,
            value.xy, value.yy, value.yz,
            value.xz, value.yz, value.zz};
    }

    ae::SymmetricTensor3 symmetricFromFull(const std::array<double, 9>& value)
    {
        return {
            value[0],
            value[4],
            value[8],
            0.5 * (value[5] + value[7]),
            0.5 * (value[2] + value[6]),
            0.5 * (value[1] + value[3])};
    }

    std::array<double, 9> multiply3(
        const std::array<double, 9>& a,
        const std::array<double, 9>& b)
    {
        std::array<double, 9> result{};
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                for (std::size_t k = 0; k < 3; ++k)
                    result[3 * i + j] += a[3 * i + k] * b[3 * k + j];
        return result;
    }

    std::array<double, 9> transpose3(const std::array<double, 9>& value)
    {
        return {
            value[0], value[3], value[6],
            value[1], value[4], value[7],
            value[2], value[5], value[8]};
    }

    double determinant3(const std::array<double, 9>& m)
    {
        return
            m[0] * (m[4] * m[8] - m[5] * m[7])
            - m[1] * (m[3] * m[8] - m[5] * m[6])
            + m[2] * (m[3] * m[7] - m[4] * m[6]);
    }

    bool positiveDefinite6(const std::array<double, 36>& matrix)
    {
        long double L[6][6]{};
        long double maxDiagonal = 0.0L;
        for (std::size_t i = 0; i < 6; ++i)
            maxDiagonal = std::max(
                maxDiagonal,
                static_cast<long double>(std::abs(matrix[6 * i + i])));
        if (!(maxDiagonal > 0.0L)) return false;

        for (std::size_t i = 0; i < 6; ++i)
        {
            for (std::size_t j = 0; j <= i; ++j)
            {
                long double sum = matrix[6 * i + j];
                for (std::size_t k = 0; k < j; ++k)
                    sum -= L[i][k] * L[j][k];

                if (i == j)
                {
                    const auto tolerance =
                        64.0L * std::numeric_limits<long double>::epsilon()
                        * maxDiagonal;
                    if (!(sum > tolerance)) return false;
                    L[i][j] = std::sqrt(sum);
                }
                else
                {
                    if (!(L[j][j] > 0.0L)) return false;
                    L[i][j] = sum / L[j][j];
                }
            }
        }
        return true;
    }

    std::array<double, 36> kelvinRotationMatrix(const ae::Rotation3& rotation)
    {
        std::array<double, 36> q{};
        const std::array<ae::SymmetricTensor3, 6> basis{
            ae::SymmetricTensor3{1.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            ae::SymmetricTensor3{0.0, 1.0, 0.0, 0.0, 0.0, 0.0},
            ae::SymmetricTensor3{0.0, 0.0, 1.0, 0.0, 0.0, 0.0},
            ae::SymmetricTensor3{0.0, 0.0, 0.0, 1.0 / std::numbers::sqrt2, 0.0, 0.0},
            ae::SymmetricTensor3{0.0, 0.0, 0.0, 0.0, 1.0 / std::numbers::sqrt2, 0.0},
            ae::SymmetricTensor3{0.0, 0.0, 0.0, 0.0, 0.0, 1.0 / std::numbers::sqrt2}};

        for (std::size_t column = 0; column < 6; ++column)
        {
            const auto rotated = basis[column].rotated(rotation).kelvin();
            for (std::size_t row = 0; row < 6; ++row)
                q[6 * row + column] = rotated[row];
        }
        return q;
    }

    std::array<double, 36> multiply6(
        const std::array<double, 36>& a,
        const std::array<double, 36>& b)
    {
        std::array<double, 36> result{};
        for (std::size_t i = 0; i < 6; ++i)
            for (std::size_t j = 0; j < 6; ++j)
                for (std::size_t k = 0; k < 6; ++k)
                    result[6 * i + j] += a[6 * i + k] * b[6 * k + j];
        return result;
    }

    std::array<double, 36> transpose6(const std::array<double, 36>& value)
    {
        std::array<double, 36> result{};
        for (std::size_t i = 0; i < 6; ++i)
            for (std::size_t j = 0; j < 6; ++j)
                result[6 * i + j] = value[6 * j + i];
        return result;
    }
}

namespace ae
{
    SymmetricTensor3 SymmetricTensor3::identity(double scale)
    {
        if (!std::isfinite(scale))
            throw std::invalid_argument("Symmetric-tensor identity scale must be finite.");
        return {scale, scale, scale, 0.0, 0.0, 0.0};
    }

    SymmetricTensor3 SymmetricTensor3::fromKelvin(
        const std::array<double, 6>& values)
    {
        for (const auto value : values)
            if (!std::isfinite(value))
                throw std::invalid_argument("Kelvin tensor components must be finite.");
        return {
            values[0],
            values[1],
            values[2],
            values[3] / std::numbers::sqrt2,
            values[4] / std::numbers::sqrt2,
            values[5] / std::numbers::sqrt2};
    }

    std::array<double, 6> SymmetricTensor3::kelvin() const
    {
        const std::array<double, 6> values{
            xx, yy, zz,
            std::numbers::sqrt2 * yz,
            std::numbers::sqrt2 * xz,
            std::numbers::sqrt2 * xy};
        for (const auto value : values)
            if (!std::isfinite(value))
                throw std::invalid_argument("Symmetric tensor contains nonfinite component.");
        return values;
    }

    double SymmetricTensor3::trace() const
    {
        return xx + yy + zz;
    }

    SymmetricTensor3 SymmetricTensor3::deviator() const
    {
        const auto mean = trace() / 3.0;
        return {xx - mean, yy - mean, zz - mean, yz, xz, xy};
    }

    double SymmetricTensor3::inner(const SymmetricTensor3& other) const
    {
        return
            xx * other.xx + yy * other.yy + zz * other.zz
            + 2.0 * (yz * other.yz + xz * other.xz + xy * other.xy);
    }

    double SymmetricTensor3::norm() const
    {
        const auto squared = inner(*this);
        if (!std::isfinite(squared) || squared < 0.0)
            throw std::overflow_error("Symmetric-tensor norm is not representable.");
        return std::sqrt(squared);
    }

    SymmetricTensor3 SymmetricTensor3::rotated(const Rotation3& rotation) const
    {
        rotation.validate();
        const auto R = rotation.matrix();
        const auto result = multiply3(
            multiply3(R, fullMatrix(*this)),
            transpose3(R));
        return symmetricFromFull(result);
    }

    SymmetricTensor3 SymmetricTensor3::operator+(
        const SymmetricTensor3& other) const
    {
        return {
            xx + other.xx,
            yy + other.yy,
            zz + other.zz,
            yz + other.yz,
            xz + other.xz,
            xy + other.xy};
    }

    SymmetricTensor3 SymmetricTensor3::operator-(
        const SymmetricTensor3& other) const
    {
        return {
            xx - other.xx,
            yy - other.yy,
            zz - other.zz,
            yz - other.yz,
            xz - other.xz,
            xy - other.xy};
    }

    SymmetricTensor3 SymmetricTensor3::operator*(double scalar) const
    {
        if (!std::isfinite(scalar))
            throw std::invalid_argument("Symmetric-tensor scale must be finite.");
        return {
            scalar * xx,
            scalar * yy,
            scalar * zz,
            scalar * yz,
            scalar * xz,
            scalar * xy};
    }

    Rotation3::Rotation3(std::array<double, 9> rowMajor)
        : matrix_(rowMajor)
    {
        validate();
    }

    Rotation3 Rotation3::identity()
    {
        return Rotation3({
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0});
    }

    Rotation3 Rotation3::fromAxisAngle(
        std::array<double, 3> axis,
        double radians)
    {
        axis = normalized(axis);
        if (!std::isfinite(radians))
            throw std::invalid_argument("Rotation angle must be finite.");

        const auto c = std::cos(radians);
        const auto s = std::sin(radians);
        const auto oneMinusC = 1.0 - c;
        const auto x = axis[0];
        const auto y = axis[1];
        const auto z = axis[2];

        return Rotation3({
            c + x * x * oneMinusC,
            x * y * oneMinusC - z * s,
            x * z * oneMinusC + y * s,

            y * x * oneMinusC + z * s,
            c + y * y * oneMinusC,
            y * z * oneMinusC - x * s,

            z * x * oneMinusC - y * s,
            z * y * oneMinusC + x * s,
            c + z * z * oneMinusC});
    }

    Rotation3 Rotation3::fromLayerNormal(
        std::array<double, 3> layerNormal)
    {
        const auto n = normalized(layerNormal);
        const std::array<double, 3> ex{1.0, 0.0, 0.0};
        const std::array<double, 3> ey{0.0, 1.0, 0.0};
        const std::array<double, 3> ez{0.0, 0.0, 1.0};

        auto seed = ex;
        const auto ax = std::abs(dot(n, ex));
        const auto ay = std::abs(dot(n, ey));
        const auto az = std::abs(dot(n, ez));
        if (ay <= ax && ay <= az) seed = ey;
        else if (az <= ax && az <= ay) seed = ez;

        auto t1 = normalized(cross(seed, n));
        auto t2 = normalized(cross(n, t1));

        return Rotation3({
            t1[0], t2[0], n[0],
            t1[1], t2[1], n[1],
            t1[2], t2[2], n[2]});
    }

    double Rotation3::at(std::size_t row, std::size_t column) const
    {
        if (row >= 3 || column >= 3)
            throw std::out_of_range("Rotation3 index out of range.");
        return matrix_[3 * row + column];
    }

    const std::array<double, 9>& Rotation3::matrix() const
    {
        return matrix_;
    }

    Rotation3 Rotation3::transposed() const
    {
        return Rotation3(transpose3(matrix_));
    }

    void Rotation3::validate() const
    {
        for (const auto value : matrix_)
            if (!std::isfinite(value))
                throw std::invalid_argument("Rotation matrix contains nonfinite entry.");

        const auto gram = multiply3(transpose3(matrix_), matrix_);
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
            {
                const auto expected = i == j ? 1.0 : 0.0;
                if (std::abs(gram[3 * i + j] - expected) > kRotationTolerance)
                    throw std::invalid_argument("Rotation matrix must be orthonormal.");
            }

        if (std::abs(determinant3(matrix_) - 1.0) > 4.0 * kRotationTolerance)
            throw std::invalid_argument("Rotation matrix must be proper with determinant +1.");
    }

    KelvinElasticityTensor::KelvinElasticityTensor(
        std::array<double, 36> rowMajor)
        : matrix_(rowMajor)
    {
        validate();
    }

    KelvinElasticityTensor KelvinElasticityTensor::isotropic(
        double bulkModulus,
        double shearModulus)
    {
        IsotropicLinearElasticity material{bulkModulus, shearModulus};
        material.validate();

        const auto lambda = material.lameLambda();
        const auto normalDiagonal = lambda + 2.0 * shearModulus;
        const auto shearDiagonal = 2.0 * shearModulus;

        std::array<double, 36> matrix{};
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                matrix[6 * i + j] = i == j ? normalDiagonal : lambda;
        matrix[6 * 3 + 3] = shearDiagonal;
        matrix[6 * 4 + 4] = shearDiagonal;
        matrix[6 * 5 + 5] = shearDiagonal;
        return KelvinElasticityTensor(matrix);
    }

    double KelvinElasticityTensor::at(std::size_t row, std::size_t column) const
    {
        if (row >= 6 || column >= 6)
            throw std::out_of_range("Kelvin elasticity index out of range.");
        return matrix_[6 * row + column];
    }

    const std::array<double, 36>& KelvinElasticityTensor::matrix() const
    {
        return matrix_;
    }

    SymmetricTensor3 KelvinElasticityTensor::stress(
        const SymmetricTensor3& strain) const
    {
        const auto e = strain.kelvin();
        std::array<double, 6> s{};
        for (std::size_t i = 0; i < 6; ++i)
            for (std::size_t j = 0; j < 6; ++j)
                s[i] += matrix_[6 * i + j] * e[j];
        return SymmetricTensor3::fromKelvin(s);
    }

    double KelvinElasticityTensor::energyDensity(
        const SymmetricTensor3& strain) const
    {
        const auto stressValue = stress(strain);
        const auto energy = 0.5 * strain.inner(stressValue);
        if (!std::isfinite(energy))
            throw std::overflow_error("Elastic energy density is not representable.");
        return energy;
    }

    KelvinElasticityTensor KelvinElasticityTensor::rotated(
        const Rotation3& rotation) const
    {
        const auto q = kelvinRotationMatrix(rotation);
        auto result = multiply6(multiply6(q, matrix_), transpose6(q));
        for (std::size_t i = 0; i < 6; ++i)
            for (std::size_t j = i + 1; j < 6; ++j)
            {
                const auto average = 0.5 * (result[6 * i + j] + result[6 * j + i]);
                result[6 * i + j] = average;
                result[6 * j + i] = average;
            }
        return KelvinElasticityTensor(result);
    }

    KelvinElasticityTensor KelvinElasticityTensor::operator+(
        const KelvinElasticityTensor& other) const
    {
        std::array<double, 36> result{};
        for (std::size_t i = 0; i < result.size(); ++i)
            result[i] = matrix_[i] + other.matrix_[i];
        return KelvinElasticityTensor(result);
    }

    KelvinElasticityTensor KelvinElasticityTensor::operator*(double scalar) const
    {
        if (!std::isfinite(scalar) || scalar <= 0.0)
            throw std::invalid_argument(
                "Elasticity-tensor scale must be finite and positive.");
        std::array<double, 36> result{};
        for (std::size_t i = 0; i < result.size(); ++i)
            result[i] = scalar * matrix_[i];
        return KelvinElasticityTensor(result);
    }

    bool KelvinElasticityTensor::approximatelyEqual(
        const KelvinElasticityTensor& other,
        double relativeTolerance) const
    {
        if (!std::isfinite(relativeTolerance) || relativeTolerance < 0.0)
            throw std::invalid_argument("Elasticity comparison tolerance must be finite and nonnegative.");
        double scale = std::numeric_limits<double>::min();
        for (std::size_t i = 0; i < matrix_.size(); ++i)
            scale = std::max({
                scale,
                std::abs(matrix_[i]),
                std::abs(other.matrix_[i])});

        const auto tolerance = relativeTolerance * scale;
        for (std::size_t i = 0; i < matrix_.size(); ++i)
            if (std::abs(matrix_[i] - other.matrix_[i]) > tolerance)
                return false;
        return true;
    }

    void KelvinElasticityTensor::validate() const
    {
        double scale = 0.0;
        for (const auto value : matrix_)
        {
            if (!std::isfinite(value))
                throw std::invalid_argument("Kelvin elasticity tensor contains nonfinite entry.");
            scale = std::max(scale, std::abs(value));
        }
        if (!(scale > 0.0))
            throw std::invalid_argument("Kelvin elasticity tensor must be nonzero.");

        const auto symmetryTolerance = kSymmetryTolerance * scale;
        for (std::size_t i = 0; i < 6; ++i)
            for (std::size_t j = i + 1; j < 6; ++j)
                if (std::abs(matrix_[6 * i + j] - matrix_[6 * j + i])
                    > symmetryTolerance)
                    throw std::invalid_argument(
                        "Energy-based Kelvin elasticity tensor must be major-symmetric.");

        if (!positiveDefinite6(matrix_))
            throw std::invalid_argument(
                "M8B benchmark Kelvin elasticity tensor must have positive elastic energy.");
    }

    void IsotropicLinearElasticity::validate() const
    {
        if (!std::isfinite(bulkModulus) || bulkModulus <= 0.0
            || !std::isfinite(shearModulus) || shearModulus <= 0.0)
            throw std::invalid_argument(
                "Isotropic benchmark bulk and shear moduli must be finite and positive.");
    }

    double IsotropicLinearElasticity::lameLambda() const
    {
        validate();
        return bulkModulus - 2.0 * shearModulus / 3.0;
    }

    double IsotropicLinearElasticity::youngModulus() const
    {
        validate();
        const auto value =
            bulkModulus >= shearModulus
            ? 9.0 * shearModulus
                / (3.0 + shearModulus / bulkModulus)
            : 9.0 * bulkModulus
                / (3.0 * bulkModulus / shearModulus + 1.0);
        if (!std::isfinite(value) || value <= 0.0)
            throw std::overflow_error("Derived Young's modulus is not representable.");
        return value;
    }

    double IsotropicLinearElasticity::poissonRatio() const
    {
        validate();
        const auto scale = std::max(bulkModulus, shearModulus);
        const auto k = bulkModulus / scale;
        const auto g = shearModulus / scale;
        const auto value =
            (3.0 * k - 2.0 * g)
            / (2.0 * (3.0 * k + g));
        if (!std::isfinite(value))
            throw std::overflow_error("Derived Poisson ratio is not representable.");
        return value;
    }

    KelvinElasticityTensor IsotropicLinearElasticity::tensor() const
    {
        validate();
        return KelvinElasticityTensor::isotropic(bulkModulus, shearModulus);
    }
} // namespace ae
