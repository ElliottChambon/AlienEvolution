#include "alien_evolution/genetics/RegulatoryDynamics.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace ae
{

    namespace
    {

        void validateState(
            const RegulatoryProgram& program,
            const RegulatoryState& state
        )
        {
            if (
                state.size()
                != program.nodeCount()
                )
            {
                throw std::invalid_argument(
                    "Regulatory state size does not match program node count."
                );
            }

            for (const double value : state)
            {
                if (
                    !std::isfinite(value)
                    || value < 0.0
                    )
                {
                    throw std::invalid_argument(
                        "Regulatory activities must be finite and nonnegative."
                    );
                }
            }
        }


        std::size_t nodeIndex(
            const RegulatoryProgram& program,
            const std::uint64_t nodeId
        )
        {
            const auto& nodes =
                program.nodes();

            for (
                std::size_t i = 0;
                i < nodes.size();
                ++i
                )
            {
                if (nodes[i].id == nodeId)
                {
                    return i;
                }
            }

            throw std::logic_error(
                "Regulatory node ID could not be resolved."
            );
        }


        RegulatoryState offsetState(
            const RegulatoryState& state,
            const RegulatoryState& derivative,
            const double scale
        )
        {
            RegulatoryState result =
                state;

            for (
                std::size_t i = 0;
                i < result.size();
                ++i
                )
            {
                result[i] =
                    std::max(
                        0.0,
                        state[i]
                        + scale * derivative[i]
                    );
            }

            return result;
        }


        std::vector<bool> validateAndBuildClampMask(
            const RegulatoryState& state,
            const std::vector<RegulatoryStateClamp>& clamps
        )
        {
            std::vector<bool> mask(
                state.size(),
                false
            );

            for (const RegulatoryStateClamp& clamp : clamps)
            {
                if (clamp.index >= state.size())
                {
                    throw std::invalid_argument(
                        "Regulatory state clamp index is out of range."
                    );
                }

                if (
                    !std::isfinite(clamp.value)
                    || clamp.value < 0.0
                    )
                {
                    throw std::invalid_argument(
                        "Regulatory state clamp value must be finite and nonnegative."
                    );
                }

                if (mask[clamp.index])
                {
                    throw std::invalid_argument(
                        "A regulatory state component cannot be clamped more than once."
                    );
                }

                mask[clamp.index] =
                    true;
            }

            return mask;
        }


        void applyClamps(
            RegulatoryState& state,
            const std::vector<RegulatoryStateClamp>& clamps
        )
        {
            for (const RegulatoryStateClamp& clamp : clamps)
            {
                state[clamp.index] =
                    clamp.value;
            }
        }


        RegulatoryState calculateDerivatives(
            const RegulatoryProgram& program,
            const RegulatoryState& state,
            const std::vector<bool>* clampedMask
        )
        {
            validateState(
                program,
                state
            );

            RegulatoryState derivative(
                program.nodeCount(),
                0.0
            );

            const auto& nodes =
                program.nodes();

            const auto& interactions =
                program.interactions();

            for (
                std::size_t targetIndex = 0;
                targetIndex < nodes.size();
                ++targetIndex
                )
            {
                // A clamped component is controlled by an external boundary
                // condition. Its intrinsic production/degradation equation
                // therefore does not participate in this integration step.
                if (
                    clampedMask != nullptr
                    && (*clampedMask)[targetIndex]
                    )
                {
                    derivative[targetIndex] =
                        0.0;

                    continue;
                }

                const RegulatoryNode& target =
                    nodes[targetIndex];

                double production =
                    target.basalProductionRate;

                for (
                    const RegulatoryInteraction& interaction :
                    interactions
                    )
                {
                    if (
                        interaction.targetNodeId
                        != target.id
                        )
                    {
                        continue;
                    }

                    const std::size_t sourceIndex =
                        nodeIndex(
                            program,
                            interaction.sourceNodeId
                        );

                    const double regulatoryEffect =
                        RegulatoryDynamics::shiftedHill(
                            state[sourceIndex],
                            interaction.halfSaturation,
                            interaction.cooperativity,
                            interaction.foldChange
                        );

                    production *=
                        regulatoryEffect;
                }

                if (!std::isfinite(production))
                {
                    throw std::overflow_error(
                        "Regulatory production became non-finite."
                    );
                }

                derivative[targetIndex] =
                    production
                    -
                    target.degradationRate
                    * state[targetIndex];
            }

            return derivative;
        }

    } // namespace


    double RegulatoryDynamics::shiftedHill(
        const double activity,
        const double halfSaturation,
        const double cooperativity,
        const double foldChange
    )
    {
        if (
            !std::isfinite(activity)
            || activity < 0.0
            )
        {
            throw std::invalid_argument(
                "Regulatory activity must be finite and nonnegative."
            );
        }

        if (
            !std::isfinite(halfSaturation)
            || halfSaturation <= 0.0
            )
        {
            throw std::invalid_argument(
                "Half-saturation must be finite and positive."
            );
        }

        if (
            !std::isfinite(cooperativity)
            || cooperativity <= 0.0
            )
        {
            throw std::invalid_argument(
                "Cooperativity must be finite and positive."
            );
        }

        if (
            !std::isfinite(foldChange)
            || foldChange <= 0.0
            )
        {
            throw std::invalid_argument(
                "Fold change must be finite and positive."
            );
        }

        const double normalizedActivity =
            activity / halfSaturation;

        const double ratio =
            std::pow(
                normalizedActivity,
                cooperativity
            );

        const double engagedFraction =
            std::isinf(ratio)
            ? 1.0
            : ratio / (1.0 + ratio);

        return
            1.0
            + (foldChange - 1.0)
            * engagedFraction;
    }


    RegulatoryState RegulatoryDynamics::initialState(
        const RegulatoryProgram& program
    )
    {
        RegulatoryState state;

        state.reserve(
            program.nodeCount()
        );

        for (
            const RegulatoryNode& node :
            program.nodes()
            )
        {
            state.push_back(
                node.initialActivity
            );
        }

        return state;
    }


    RegulatoryState RegulatoryDynamics::derivatives(
        const RegulatoryProgram& program,
        const RegulatoryState& state
    )
    {
        return calculateDerivatives(
            program,
            state,
            nullptr
        );
    }


    RegulatoryState RegulatoryDynamics::stepRK4(
        const RegulatoryProgram& program,
        const RegulatoryState& state,
        const double timeStep
    )
    {
        return stepRK4(
            program,
            state,
            timeStep,
            {}
        );
    }


    RegulatoryState RegulatoryDynamics::stepRK4(
        const RegulatoryProgram& program,
        const RegulatoryState& state,
        const double timeStep,
        const std::vector<RegulatoryStateClamp>& clamps
    )
    {
        validateState(
            program,
            state
        );

        if (
            !std::isfinite(timeStep)
            || timeStep <= 0.0
            )
        {
            throw std::invalid_argument(
                "Regulatory integration time step must be finite and positive."
            );
        }

        const std::vector<bool> clampedMask =
            validateAndBuildClampMask(
                state,
                clamps
            );


        // --------------------------------------------------------
        // Stage 1
        // --------------------------------------------------------

        RegulatoryState state1 =
            state;

        applyClamps(
            state1,
            clamps
        );

        const RegulatoryState k1 =
            calculateDerivatives(
                program,
                state1,
                &clampedMask
            );


        // --------------------------------------------------------
        // Stage 2
        // --------------------------------------------------------

        RegulatoryState state2 =
            offsetState(
                state1,
                k1,
                0.5 * timeStep
            );

        applyClamps(
            state2,
            clamps
        );

        const RegulatoryState k2 =
            calculateDerivatives(
                program,
                state2,
                &clampedMask
            );


        // --------------------------------------------------------
        // Stage 3
        // --------------------------------------------------------

        RegulatoryState state3 =
            offsetState(
                state1,
                k2,
                0.5 * timeStep
            );

        applyClamps(
            state3,
            clamps
        );

        const RegulatoryState k3 =
            calculateDerivatives(
                program,
                state3,
                &clampedMask
            );


        // --------------------------------------------------------
        // Stage 4
        // --------------------------------------------------------

        RegulatoryState state4 =
            offsetState(
                state1,
                k3,
                timeStep
            );

        applyClamps(
            state4,
            clamps
        );

        const RegulatoryState k4 =
            calculateDerivatives(
                program,
                state4,
                &clampedMask
            );


        // --------------------------------------------------------
        // Final RK4 combination
        // --------------------------------------------------------

        RegulatoryState nextState =
            state1;

        for (
            std::size_t i = 0;
            i < nextState.size();
            ++i
            )
        {
            if (clampedMask[i])
            {
                continue;
            }

            const double increment =
                (timeStep / 6.0)
                * (
                    k1[i]
                    + 2.0 * k2[i]
                    + 2.0 * k3[i]
                    + k4[i]
                    );

            nextState[i] =
                std::max(
                    0.0,
                    state1[i] + increment
                );

            if (!std::isfinite(nextState[i]))
            {
                throw std::overflow_error(
                    "Regulatory activity became non-finite."
                );
            }
        }

        applyClamps(
            nextState,
            clamps
        );

        return nextState;
    }


    RegulatoryState RegulatoryDynamics::simulate(
        const RegulatoryProgram& program,
        RegulatoryState state,
        const double duration,
        const double timeStep
    )
    {
        validateState(
            program,
            state
        );

        if (
            !std::isfinite(duration)
            || duration < 0.0
            )
        {
            throw std::invalid_argument(
                "Simulation duration must be finite and nonnegative."
            );
        }

        if (
            !std::isfinite(timeStep)
            || timeStep <= 0.0
            )
        {
            throw std::invalid_argument(
                "Simulation time step must be finite and positive."
            );
        }

        double elapsed =
            0.0;

        while (elapsed < duration)
        {
            const double remaining =
                duration - elapsed;

            const double step =
                std::min(
                    timeStep,
                    remaining
                );

            if (step <= 0.0)
            {
                break;
            }

            state =
                stepRK4(
                    program,
                    state,
                    step
                );

            elapsed +=
                step;
        }

        return state;
    }

} // namespace ae