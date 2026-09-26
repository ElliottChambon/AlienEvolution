#include "alien_evolution/genetics/RegulatoryMutationSampler.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace ae
{

    namespace
    {

        void validateMean(
            const double mean
        )
        {
            if (
                !std::isfinite(mean)
                || mean < 0.0
                )
            {
                throw std::invalid_argument(
                    "Poisson mutation mean must be finite and nonnegative."
                );
            }
        }

        double openUniform01(
            Random& random
        )
        {
            // Random::uniform01() can legitimately return zero.
            // Logarithms used by the rejection sampler require
            // a strictly positive value.
            return std::max(
                random.uniform01(),
                std::numeric_limits<double>::min()
            );
        }

        // Exact waiting-time / product method.
        //
        // Efficient for small Poisson means.
        // Runtime scales approximately with the sampled count.
        std::uint64_t samplePoissonSmall(
            const double mean,
            Random& random
        )
        {
            const double threshold =
                std::exp(-mean);

            double product = 1.0;

            std::uint64_t count = 0;

            do
            {
                ++count;

                product *=
                    openUniform01(random);
            } while (product > threshold);

            return count - 1;
        }

        // Hörmann-style transformed rejection sampler.
        //
        // This is used for larger Poisson means so runtime does not
        // grow linearly with the expected mutation count.
        std::uint64_t samplePoissonLarge(
            const double mean,
            Random& random
        )
        {
            const double sqrtMean =
                std::sqrt(mean);

            const double b =
                0.931
                + 2.53 * sqrtMean;

            const double a =
                -0.059
                + 0.02483 * b;

            const double inverseAlpha =
                1.1239
                + 1.1328 / (b - 3.4);

            const double squeeze =
                0.9277
                - 3.6224 / (b - 2.0);

            while (true)
            {
                const double u =
                    random.uniform01()
                    - 0.5;

                const double v =
                    openUniform01(random);

                const double us =
                    0.5
                    - std::abs(u);

                if (us <= 0.0)
                {
                    continue;
                }

                const double candidateReal =
                    (
                        2.0 * a / us
                        + b
                        )
                    * u
                    + mean
                    + 0.43;

                const double floored =
                    std::floor(candidateReal);

                if (floored < 0.0)
                {
                    continue;
                }

                if (
                    floored
                > static_cast<double>(
                    std::numeric_limits<std::uint64_t>::max()
                    )
                    )
                {
                    throw std::overflow_error(
                        "Sampled Poisson mutation count exceeds integer range."
                    );
                }

                const std::uint64_t candidate =
                    static_cast<std::uint64_t>(
                        floored
                        );

                if (
                    us >= 0.07
                    && v <= squeeze
                    )
                {
                    return candidate;
                }

                if (
                    us < 0.013
                    && v > us
                    )
                {
                    continue;
                }

                const double candidateDouble =
                    static_cast<double>(
                        candidate
                        );

                const double left =
                    std::log(v)
                    + std::log(inverseAlpha)
                    - std::log(
                        a / (us * us)
                        + b
                    );

                const double right =
                    -mean
                    + candidateDouble
                    * std::log(mean)
                    - std::lgamma(
                        candidateDouble + 1.0
                    );

                if (left <= right)
                {
                    return candidate;
                }
            }
        }

        std::uint64_t samplePoisson(
            const double mean,
            Random& random
        )
        {
            validateMean(mean);

            if (mean == 0.0)
            {
                return 0;
            }

            // The product method is simple and efficient at small means.
            // The transformed-rejection algorithm is used above this
            // numerical threshold for computational efficiency.
            //
            // The threshold is an implementation choice, not biology.
            if (mean < 10.0)
            {
                return samplePoissonSmall(
                    mean,
                    random
                );
            }

            return samplePoissonLarge(
                mean,
                random
            );
        }

        std::uint64_t checkedAdd(
            const std::uint64_t a,
            const std::uint64_t b
        )
        {
            if (
                b
            > std::numeric_limits<std::uint64_t>::max()
                - a
                )
            {
                throw std::overflow_error(
                    "Total regulatory mutation count overflowed."
                );
            }

            return a + b;
        }

    } // namespace

    std::uint64_t RegulatoryMutationCounts::total() const
    {
        std::uint64_t result = 0;

        result =
            checkedAdd(
                result,
                nodeKinetic
            );

        result =
            checkedAdd(
                result,
                interactionParameter
            );

        result =
            checkedAdd(
                result,
                interactionGain
            );

        result =
            checkedAdd(
                result,
                interactionLoss
            );

        result =
            checkedAdd(
                result,
                regulatoryUnitDuplication
            );

        result =
            checkedAdd(
                result,
                regulatoryNodeLoss
            );

        return result;
    }

    RegulatoryMutationCounts sampleRegulatoryMutationCounts(
        const RegulatoryMutationHazards& hazards,
        Random& random
    )
    {
        RegulatoryMutationCounts counts{};

        counts.nodeKinetic =
            samplePoisson(
                hazards.expectedNodeKineticEvents,
                random
            );

        counts.interactionParameter =
            samplePoisson(
                hazards.expectedInteractionParameterEvents,
                random
            );

        counts.interactionGain =
            samplePoisson(
                hazards.expectedInteractionGainEvents,
                random
            );

        counts.interactionLoss =
            samplePoisson(
                hazards.expectedInteractionLossEvents,
                random
            );

        counts.regulatoryUnitDuplication =
            samplePoisson(
                hazards.expectedDuplicationEvents,
                random
            );

        counts.regulatoryNodeLoss =
            samplePoisson(
                hazards.expectedNodeLossEvents,
                random
            );

        return counts;
    }

} // namespace ae