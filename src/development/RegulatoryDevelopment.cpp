#include "alien_evolution/development/RegulatoryDevelopment.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ae
{

    RegulatoryDevelopment::RegulatoryDevelopment(
        const std::size_t width,
        const std::size_t height,
        const std::size_t steps,
        RegulatoryDevelopmentConfig config
    )
        : width_(width),
        height_(height),
        steps_(steps),
        config_(std::move(config))
    {
        if (
            width_ == 0
            || height_ == 0
            )
        {
            throw std::invalid_argument(
                "Regulatory development dimensions must be positive."
            );
        }

        if (
            !std::isfinite(
                config_.neighborhoodLengthScale
            )
            || config_.neighborhoodLengthScale <= 0.0
            )
        {
            throw std::invalid_argument(
                "Neighborhood length scale must be finite and positive."
            );
        }

        if (
            !std::isfinite(
                config_.regulatoryTimeStep
            )
            || config_.regulatoryTimeStep <= 0.0
            )
        {
            throw std::invalid_argument(
                "Regulatory time step must be finite and positive."
            );
        }

        if (
            config_.regulatoryStepsPerDevelopmentStep == 0
            )
        {
            throw std::invalid_argument(
                "At least one regulatory integration step is required."
            );
        }

        if (
            !std::isfinite(
                config_.outputHalfSaturation
            )
            || config_.outputHalfSaturation <= 0.0
            )
        {
            throw std::invalid_argument(
                "Output half-saturation must be finite and positive."
            );
        }

        if (
            !std::isfinite(
                config_.outputCooperativity
            )
            || config_.outputCooperativity <= 0.0
            )
        {
            throw std::invalid_argument(
                "Output cooperativity must be finite and positive."
            );
        }

        if (
            !std::isfinite(
                config_.depositionRateScale
            )
            || config_.depositionRateScale < 0.0
            )
        {
            throw std::invalid_argument(
                "Deposition rate scale must be finite and nonnegative."
            );
        }
    }

    std::size_t RegulatoryDevelopment::index(
        const std::size_t x,
        const std::size_t y
    ) const
    {
        return y * width_ + x;
    }

    std::size_t RegulatoryDevelopment::nodeIndex(
        const RegulatoryProgram& program,
        const std::uint64_t nodeId
    ) const
    {
        for (std::size_t i = 0;
            i < program.nodeCount();
            ++i)
        {
            if (program.nodes()[i].id == nodeId)
            {
                return i;
            }
        }

        throw std::invalid_argument(
            "Regulatory development interface references nonexistent node."
        );
    }

    double RegulatoryDevelopment::localMaterialSignal(
        const Phenotype& phenotype,
        const std::size_t x,
        const std::size_t y
    ) const
    {
        double weightedMaterial = 0.0;
        double totalWeight = 0.0;

        for (int offsetY = -1;
            offsetY <= 1;
            ++offsetY)
        {
            for (int offsetX = -1;
                offsetX <= 1;
                ++offsetX)
            {
                if (
                    offsetX == 0
                    && offsetY == 0
                    )
                {
                    continue;
                }

                const long long neighborX =
                    static_cast<long long>(x)
                    + offsetX;

                const long long neighborY =
                    static_cast<long long>(y)
                    + offsetY;

                if (
                    neighborX < 0
                    || neighborY < 0
                    || neighborX
                    >= static_cast<long long>(width_)
                    || neighborY
                    >= static_cast<long long>(height_)
                    )
                {
                    continue;
                }

                const double distance =
                    std::sqrt(
                        static_cast<double>(
                            offsetX * offsetX
                            + offsetY * offsetY
                            )
                    );

                const double weight =
                    std::exp(
                        -distance
                        / config_.neighborhoodLengthScale
                    );

                weightedMaterial +=
                    weight
                    * phenotype.materialAt(
                        static_cast<std::size_t>(
                            neighborX
                            ),
                        static_cast<std::size_t>(
                            neighborY
                            )
                    );

                totalWeight +=
                    weight;
            }
        }

        if (totalWeight == 0.0)
        {
            return 0.0;
        }

        return weightedMaterial
            / totalWeight;
    }

    RegulatoryState RegulatoryDevelopment::inheritedState(
        const Phenotype& phenotype,
        const std::vector<RegulatoryState>& states,
        const RegulatoryProgram& program,
        const std::size_t x,
        const std::size_t y
    ) const
    {
        const double currentMaterial =
            phenotype.materialAt(
                x,
                y
            );

        if (currentMaterial > 0.0)
        {
            return states[
                index(
                    x,
                    y
                )
            ];
        }

        RegulatoryState inherited(
            program.nodeCount(),
            0.0
        );

        double totalWeight = 0.0;

        for (int offsetY = -1;
            offsetY <= 1;
            ++offsetY)
        {
            for (int offsetX = -1;
                offsetX <= 1;
                ++offsetX)
            {
                if (
                    offsetX == 0
                    && offsetY == 0
                    )
                {
                    continue;
                }

                const long long neighborX =
                    static_cast<long long>(x)
                    + offsetX;

                const long long neighborY =
                    static_cast<long long>(y)
                    + offsetY;

                if (
                    neighborX < 0
                    || neighborY < 0
                    || neighborX
                    >= static_cast<long long>(width_)
                    || neighborY
                    >= static_cast<long long>(height_)
                    )
                {
                    continue;
                }

                const std::size_t nx =
                    static_cast<std::size_t>(
                        neighborX
                        );

                const std::size_t ny =
                    static_cast<std::size_t>(
                        neighborY
                        );

                const double material =
                    phenotype.materialAt(
                        nx,
                        ny
                    );

                if (material <= 0.0)
                {
                    continue;
                }

                const double distance =
                    std::sqrt(
                        static_cast<double>(
                            offsetX * offsetX
                            + offsetY * offsetY
                            )
                    );

                const double spatialWeight =
                    std::exp(
                        -distance
                        / config_.neighborhoodLengthScale
                    );

                const double weight =
                    spatialWeight
                    * material;

                const RegulatoryState& neighborState =
                    states[
                        index(
                            nx,
                            ny
                        )
                    ];

                for (std::size_t node = 0;
                    node < inherited.size();
                    ++node)
                {
                    inherited[node] +=
                        weight
                        * neighborState[node];
                }

                totalWeight +=
                    weight;
            }
        }

        if (totalWeight == 0.0)
        {
            return RegulatoryDynamics::initialState(
                program
            );
        }

        for (double& activity :
            inherited)
        {
            activity /=
                totalWeight;
        }

        return inherited;
    }

    Phenotype RegulatoryDevelopment::develop(
        const RegulatoryProgram& program,
        const Environment& environment
    ) const
    {
        const std::size_t localInputIndex =
            nodeIndex(
                program,
                config_.localMaterialInputNodeId
            );

        const std::size_t resourceInputIndex =
            nodeIndex(
                program,
                config_.resourceInputNodeId
            );

        const std::size_t outputIndex =
            nodeIndex(
                program,
                config_.depositionOutputNodeId
            );

        // For this V0.2 interface we keep the sensing nodes
        // distinct from the effector-output node.
        //
        // This is an interface simplification, not a universal
        // biological claim.
        if (
            outputIndex == localInputIndex
            || outputIndex == resourceInputIndex
            )
        {
            throw std::invalid_argument(
                "V0.2 regulatory development requires output node to be distinct from input nodes."
            );
        }

        Phenotype phenotype(
            width_,
            height_
        );

        const std::size_t centerX =
            width_ / 2;

        const std::size_t centerY =
            height_ / 2;

        phenotype.setMaterial(
            centerX,
            centerY,
            1.0
        );

        const RegulatoryState initialState =
            RegulatoryDynamics::initialState(
                program
            );

        std::vector<RegulatoryState> states(
            width_ * height_,
            initialState
        );

        const double resourceSignal =
            std::max(
                environment.resourceAvailability,
                0.0
            );

        for (std::size_t developmentalStep = 0;
            developmentalStep < steps_;
            ++developmentalStep)
        {
            Phenotype nextPhenotype =
                phenotype;

            std::vector<RegulatoryState> nextStates =
                states;

            for (std::size_t y = 0;
                y < height_;
                ++y)
            {
                for (std::size_t x = 0;
                    x < width_;
                    ++x)
                {
                    const double currentMaterial =
                        phenotype.materialAt(
                            x,
                            y
                        );

                    const double localSignal =
                        localMaterialSignal(
                            phenotype,
                            x,
                            y
                        );

                    // Empty locations completely disconnected from
                    // existing material are not developmental units.
                    if (
                        currentMaterial <= 0.0
                        && localSignal <= 0.0
                        )
                    {
                        continue;
                    }

                    RegulatoryState state =
                        inheritedState(
                            phenotype,
                            states,
                            program,
                            x,
                            y
                        );

                    // Physical signals are imposed on designated
                    // regulatory input nodes.
                    state[localInputIndex] =
                        localSignal;

                    state[resourceInputIndex] =
                        resourceSignal;

                    // V0.2 approximation:
                    //
                    // Input activities are re-imposed after every RK4
                    // step, approximating continuously clamped external
                    // signals.
                    //
                    // A future dynamics backend should support true
                    // boundary conditions inside every RK stage.
                    for (
                        std::size_t regulatoryStep = 0;
                        regulatoryStep
                        < config_.regulatoryStepsPerDevelopmentStep;
                        ++regulatoryStep
                        )
                    {
                        state =
                            RegulatoryDynamics::stepRK4(
                                program,
                                state,
                                config_.regulatoryTimeStep
                            );

                        state[localInputIndex] =
                            localSignal;

                        state[resourceInputIndex] =
                            resourceSignal;
                    }

                    const double outputActivity =
                        std::max(
                            state[outputIndex],
                            0.0
                        );

                    // shiftedHill(..., foldChange = 2) ranges from
                    // 1 to 2, so subtracting 1 gives a bounded
                    // occupancy/readout between 0 and 1.
                    const double effectorActivation =
                        RegulatoryDynamics::shiftedHill(
                            outputActivity,
                            config_.outputHalfSaturation,
                            config_.outputCooperativity,
                            2.0
                        )
                        - 1.0;

                    const double deposition =
                        config_.depositionRateScale
                        * effectorActivation
                        * (1.0 - currentMaterial)
                        * resourceSignal;

                    if (deposition > 0.0)
                    {
                        nextPhenotype.setMaterial(
                            x,
                            y,
                            currentMaterial
                            + deposition
                        );
                    }

                    if (
                        currentMaterial > 0.0
                        || deposition > 0.0
                        )
                    {
                        nextStates[
                            index(
                                x,
                                y
                            )
                        ] =
                            std::move(state);
                    }
                }
            }

            phenotype =
                std::move(
                    nextPhenotype
                );

            states =
                std::move(
                    nextStates
                );
        }

        return phenotype;
    }

} // namespace ae