#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/genetics/RegulatoryDynamics.hpp"
#include "alien_evolution/genetics/RegulatoryProgram.hpp"
#include "alien_evolution/development/Phenotype.hpp"

namespace ae
{

    struct RegulatoryDevelopmentConfig
    {
        // Nodes used as the interface between physical state
        // and the internal regulatory network.
        std::uint64_t localMaterialInputNodeId = 0;
        std::uint64_t resourceInputNodeId = 0;

        // Regulatory node whose activity controls the current
        // V0.2 material-deposition effector.
        std::uint64_t depositionOutputNodeId = 0;

        // Spatial scale used when sensing nearby material.
        //
        // This is a model-resolution parameter, not a biological
        // constant.
        double neighborhoodLengthScale = 1.0;

        // Regulatory integration settings.
        //
        // These are numerical parameters.
        double regulatoryTimeStep = 0.05;
        std::size_t regulatoryStepsPerDevelopmentStep = 10;

        // Converts output-node activity into a bounded [0,1]
        // effector activation.
        double outputHalfSaturation = 0.5;
        double outputCooperativity = 2.0;

        // Converts effector activation into material deposition.
        //
        // This remains an explicit V0.2 developmental abstraction.
        double depositionRateScale = 0.0;
    };

    class RegulatoryDevelopment
    {
    public:
        RegulatoryDevelopment(
            std::size_t width,
            std::size_t height,
            std::size_t steps,
            RegulatoryDevelopmentConfig config
        );

        [[nodiscard]]
        Phenotype develop(
            const RegulatoryProgram& program,
            const Environment& environment
        ) const;

    private:
        std::size_t width_;
        std::size_t height_;
        std::size_t steps_;

        RegulatoryDevelopmentConfig config_;

        [[nodiscard]]
        std::size_t index(
            std::size_t x,
            std::size_t y
        ) const;

        [[nodiscard]]
        double localMaterialSignal(
            const Phenotype& phenotype,
            std::size_t x,
            std::size_t y
        ) const;

        [[nodiscard]]
        RegulatoryState inheritedState(
            const Phenotype& phenotype,
            const std::vector<RegulatoryState>& states,
            const RegulatoryProgram& program,
            std::size_t x,
            std::size_t y
        ) const;

        [[nodiscard]]
        std::size_t nodeIndex(
            const RegulatoryProgram& program,
            std::uint64_t nodeId
        ) const;
    };

} // namespace ae