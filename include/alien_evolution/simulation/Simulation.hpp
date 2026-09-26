#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/development/RegulatoryDevelopment.hpp"
#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/evaluation/Energetics.hpp"
#include "alien_evolution/evolution/Population.hpp"
#include "alien_evolution/evolution/Reproduction.hpp"
#include "alien_evolution/genetics/RegulatoryMutationGenerator.hpp"
#include "alien_evolution/genetics/RegulatoryProgram.hpp"

namespace ae
{

    struct SimulationConfig
    {
        std::size_t populationSize =
            100;

        std::size_t developmentWidth =
            40;

        std::size_t developmentHeight =
            30;

        std::size_t developmentSteps =
            10;

        RegulatoryDevelopmentConfig development{};

        RegulatoryMutationGeneratorConfig initialVariation{};

        RegulatoryMutationGeneratorConfig offspringMutation{};

        EnergeticsConfig energetics{};

        SelectionMode selectionMode =
            SelectionMode::FitnessProportional;
    };

    struct GenerationStatistics
    {
        std::size_t generation =
            0;

        std::size_t populationSize =
            0;

        double meanFitness =
            0.0;

        double maximumFitness =
            0.0;

        double meanMaterial =
            0.0;

        double meanBoundary =
            0.0;

        double meanNetEnergy =
            0.0;

        double meanRegulatoryNodeCount =
            0.0;

        double meanRegulatoryInteractionCount =
            0.0;

        double meanRegulatoryNetworkDensity =
            0.0;

        std::size_t developmentFailureCount =
            0;

        double developmentFailureFraction =
            0.0;
    };

    class Simulation
    {
    public:
        Simulation(
            Environment environment,
            RegulatoryProgram founderProgram,
            SimulationConfig config,
            std::uint64_t seed
        );

        [[nodiscard]]
        const Population& population() const;

        [[nodiscard]]
        const Environment& environment() const;

        [[nodiscard]]
        std::size_t generation() const;

        [[nodiscard]]
        std::uint64_t seed() const;

        [[nodiscard]]
        bool extinct() const;

        [[nodiscard]]
        GenerationStatistics step();

        [[nodiscard]]
        std::vector<GenerationStatistics> run(
            std::size_t generationCount
        );

    private:
        Environment environment_;

        SimulationConfig config_;

        Random random_;

        RegulatoryDevelopment development_;

        Population population_;

        std::size_t generation_ =
            0;

        [[nodiscard]]
        GenerationStatistics evaluatePopulation();
    };

} // namespace ae