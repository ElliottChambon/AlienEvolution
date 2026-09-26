#include "alien_evolution/genetics/RegulatoryMutationEffectModel.hpp"

#include <cmath>
#include <stdexcept>

namespace ae
{

    namespace
    {

        void validateScale(
            const double scale,
            const char* message
        )
        {
            if (
                !std::isfinite(scale)
                || scale < 0.0
                )
            {
                throw std::invalid_argument(
                    message
                );
            }
        }

        double sampleLogMultiplier(
            const double logStandardDeviation,
            Random& random
        )
        {
            validateScale(
                logStandardDeviation,
                "Mutation-effect log standard deviation must be finite and nonnegative."
            );

            if (logStandardDeviation == 0.0)
            {
                return 1.0;
            }

            const double logEffect =
                random.normal(
                    0.0,
                    logStandardDeviation
                );

            const double multiplier =
                std::exp(logEffect);

            if (
                !std::isfinite(multiplier)
                || multiplier <= 0.0
                )
            {
                throw std::overflow_error(
                    "Quantitative mutation effect produced an invalid multiplier."
                );
            }

            return multiplier;
        }

        void validateModel(
            const RegulatoryMutationEffectModel& model
        )
        {
            validateScale(
                model.nodeBasalProductionLogStdDev,
                "Node basal-production effect scale must be finite and nonnegative."
            );

            validateScale(
                model.nodeDegradationLogStdDev,
                "Node degradation effect scale must be finite and nonnegative."
            );

            validateScale(
                model.interactionFoldChangeLogStdDev,
                "Interaction fold-change effect scale must be finite and nonnegative."
            );

            validateScale(
                model.interactionHalfSaturationLogStdDev,
                "Interaction half-saturation effect scale must be finite and nonnegative."
            );

            validateScale(
                model.interactionCooperativityLogStdDev,
                "Interaction cooperativity effect scale must be finite and nonnegative."
            );
        }

    } // namespace

    NodeKineticChange sampleNodeKineticChange(
        const RegulatoryMutationEffectModel& model,
        Random& random
    )
    {
        validateModel(model);

        return {
            sampleLogMultiplier(
                model.nodeBasalProductionLogStdDev,
                random
            ),

            sampleLogMultiplier(
                model.nodeDegradationLogStdDev,
                random
            )
        };
    }

    InteractionParameterChange sampleInteractionParameterChange(
        const RegulatoryMutationEffectModel& model,
        Random& random
    )
    {
        validateModel(model);

        return {
            sampleLogMultiplier(
                model.interactionFoldChangeLogStdDev,
                random
            ),

            sampleLogMultiplier(
                model.interactionHalfSaturationLogStdDev,
                random
            ),

            sampleLogMultiplier(
                model.interactionCooperativityLogStdDev,
                random
            )
        };
    }

} // namespace ae