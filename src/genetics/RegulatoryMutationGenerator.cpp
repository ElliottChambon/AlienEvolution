#include "alien_evolution/genetics/RegulatoryMutationGenerator.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

#include "alien_evolution/genetics/RegulatoryMutationTargetSelector.hpp"

namespace ae
{

    namespace
    {

        enum class MutationClass
        {
            NodeKinetic,
            InteractionParameter,
            InteractionGain,
            InteractionLoss,
            Duplication,
            NodeLoss
        };

        double openUniform01(
            Random& random
        )
        {
            const double lower =
                std::numeric_limits<double>::min();

            const double upper =
                std::nextafter(
                    1.0,
                    0.0
                );

            return std::clamp(
                random.uniform01(),
                lower,
                upper
            );
        }

        void validateNonnegativeFinite(
            const double value,
            const char* message
        )
        {
            if (
                !std::isfinite(value)
                || value < 0.0
                )
            {
                throw std::invalid_argument(
                    message
                );
            }
        }

        void validatePositiveFinite(
            const double value,
            const char* message
        )
        {
            if (
                !std::isfinite(value)
                || value <= 0.0
                )
            {
                throw std::invalid_argument(
                    message
                );
            }
        }

        void validateGainModel(
            const RegulatoryInteractionGainModel& model
        )
        {
            validateNonnegativeFinite(
                model.foldChangeLogStdDev,
                "Gain fold-change effect scale must be finite and nonnegative."
            );

            validatePositiveFinite(
                model.referenceHalfSaturation,
                "Gain reference half-saturation must be finite and positive."
            );

            validateNonnegativeFinite(
                model.halfSaturationLogStdDev,
                "Gain half-saturation effect scale must be finite and nonnegative."
            );

            validatePositiveFinite(
                model.referenceCooperativity,
                "Gain reference cooperativity must be finite and positive."
            );

            validateNonnegativeFinite(
                model.cooperativityLogStdDev,
                "Gain cooperativity effect scale must be finite and nonnegative."
            );
        }

        double sampleLogMultiplier(
            const double logStandardDeviation,
            Random& random
        )
        {
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
                    "Interaction-gain effect produced invalid multiplier."
                );
            }

            return multiplier;
        }

        RegulatoryInteraction makeGainedInteraction(
            const RegulatoryInteractionTarget& target,
            const RegulatoryInteractionGainModel& model,
            Random& random
        )
        {
            const double foldChange =
                sampleLogMultiplier(
                    model.foldChangeLogStdDev,
                    random
                );

            const double halfSaturation =
                model.referenceHalfSaturation
                * sampleLogMultiplier(
                    model.halfSaturationLogStdDev,
                    random
                );

            const double cooperativity =
                model.referenceCooperativity
                * sampleLogMultiplier(
                    model.cooperativityLogStdDev,
                    random
                );

            if (
                !std::isfinite(halfSaturation)
                || halfSaturation <= 0.0
                || !std::isfinite(cooperativity)
                || cooperativity <= 0.0
                )
            {
                throw std::overflow_error(
                    "Interaction-gain initialization produced invalid parameters."
                );
            }

            return {
                target.sourceNodeId,
                target.targetNodeId,
                foldChange,
                halfSaturation,
                cooperativity
            };
        }

        std::uint64_t nextLocalNodeId(
            const RegulatoryProgram& program
        )
        {
            std::uint64_t maximumId = 0;

            for (const RegulatoryNode& node :
                program.nodes())
            {
                maximumId =
                    std::max(
                        maximumId,
                        node.id
                    );
            }

            if (
                maximumId
                == std::numeric_limits<std::uint64_t>::max()
                )
            {
                throw std::overflow_error(
                    "No monotonically increasing regulatory node ID remains."
                );
            }

            return maximumId + 1;
        }

        MutationClass selectMutationClass(
            const RegulatoryMutationHazards& hazards,
            Random& random
        )
        {
            const double total =
                hazards.totalExpectedEvents;

            if (
                !std::isfinite(total)
                || total <= 0.0
                )
            {
                throw std::logic_error(
                    "Cannot select mutation class from zero total hazard."
                );
            }

            const double draw =
                random.uniform01()
                * total;

            double cumulative =
                hazards.expectedNodeKineticEvents;

            if (draw < cumulative)
            {
                return MutationClass::NodeKinetic;
            }

            cumulative +=
                hazards.expectedInteractionParameterEvents;

            if (draw < cumulative)
            {
                return MutationClass::InteractionParameter;
            }

            cumulative +=
                hazards.expectedInteractionGainEvents;

            if (draw < cumulative)
            {
                return MutationClass::InteractionGain;
            }

            cumulative +=
                hazards.expectedInteractionLossEvents;

            if (draw < cumulative)
            {
                return MutationClass::InteractionLoss;
            }

            cumulative +=
                hazards.expectedDuplicationEvents;

            if (draw < cumulative)
            {
                return MutationClass::Duplication;
            }

            return MutationClass::NodeLoss;
        }

        RegulatoryMutationEvent createMutationEvent(
            const MutationClass mutationClass,
            const RegulatoryProgram& currentProgram,
            const RegulatoryMutationGeneratorConfig& config,
            Random& random
        )
        {
            switch (mutationClass)
            {
            case MutationClass::NodeKinetic:
            {
                const std::uint64_t nodeId =
                    selectUniformNodeKineticTarget(
                        currentProgram,
                        random
                    );

                const NodeKineticChange change =
                    sampleNodeKineticChange(
                        config.quantitativeEffects,
                        random
                    );

                return NodeKineticMutationEvent{
                    nodeId,
                    change
                };
            }

            case MutationClass::InteractionParameter:
            {
                const RegulatoryInteractionTarget target =
                    selectUniformInteractionParameterTarget(
                        currentProgram,
                        random
                    );

                const InteractionParameterChange change =
                    sampleInteractionParameterChange(
                        config.quantitativeEffects,
                        random
                    );

                return InteractionParameterMutationEvent{
                    target.sourceNodeId,
                    target.targetNodeId,
                    change
                };
            }

            case MutationClass::InteractionGain:
            {
                const RegulatoryInteractionTarget target =
                    selectUniformInteractionGainTarget(
                        currentProgram,
                        random
                    );

                const RegulatoryInteraction interaction =
                    makeGainedInteraction(
                        target,
                        config.interactionGain,
                        random
                    );

                return InteractionGainMutationEvent{
                    interaction
                };
            }

            case MutationClass::InteractionLoss:
            {
                const RegulatoryInteractionTarget target =
                    selectUniformInteractionLossTarget(
                        currentProgram,
                        random
                    );

                return InteractionLossMutationEvent{
                    target.sourceNodeId,
                    target.targetNodeId
                };
            }

            case MutationClass::Duplication:
            {
                const std::uint64_t sourceNodeId =
                    selectUniformDuplicationTarget(
                        currentProgram,
                        random
                    );

                return RegulatoryUnitDuplicationMutationEvent{
                    sourceNodeId,
                    nextLocalNodeId(
                        currentProgram
                    )
                };
            }

            case MutationClass::NodeLoss:
            {
                const std::uint64_t nodeId =
                    selectUniformNodeLossTarget(
                        currentProgram,
                        random
                    );

                return RegulatoryNodeLossMutationEvent{
                    nodeId
                };
            }
            }

            throw std::logic_error(
                "Unknown regulatory mutation class."
            );
        }

    } // namespace

    RegulatoryMutationGenerationResult generateRegulatoryOffspring(
        const RegulatoryProgram& parent,
        const RegulatoryMutationGeneratorConfig& config,
        Random& random
    )
    {
        if (config.safetyEventLimit == 0)
        {
            throw std::invalid_argument(
                "Mutation generator safety event limit must be positive."
            );
        }

        validateGainModel(
            config.interactionGain
        );

        RegulatoryProgram currentProgram =
            parent;

        std::vector<TimedRegulatoryMutationEvent> events;

        double replicationProgress = 0.0;

        while (replicationProgress < 1.0)
        {
            const RegulatoryMutationHazards hazards =
                computeRegulatoryMutationHazards(
                    currentProgram,
                    config.rates
                );

            const double totalHazard =
                hazards.totalExpectedEvents;

            if (totalHazard == 0.0)
            {
                break;
            }

            if (
                !std::isfinite(totalHazard)
                || totalHazard < 0.0
                )
            {
                throw std::logic_error(
                    "Regulatory mutation generator received invalid total hazard."
                );
            }

            const double waitingDistance =
                -std::log(
                    openUniform01(random)
                )
                / totalHazard;

            if (
                replicationProgress
                + waitingDistance
                > 1.0
                )
            {
                break;
            }

            replicationProgress +=
                waitingDistance;

            if (
                events.size()
                >= config.safetyEventLimit
                )
            {
                throw std::runtime_error(
                    "Regulatory mutation event safety limit exceeded."
                );
            }

            const MutationClass mutationClass =
                selectMutationClass(
                    hazards,
                    random
                );

            RegulatoryMutationEvent event =
                createMutationEvent(
                    mutationClass,
                    currentProgram,
                    config,
                    random
                );

            currentProgram =
                applyRegulatoryMutationEvent(
                    currentProgram,
                    event
                );

            events.push_back(
                TimedRegulatoryMutationEvent{
                    replicationProgress,
                    std::move(event)
                }
            );
        }

        return {
            std::move(currentProgram),
            std::move(events)
        };
    }

} // namespace ae