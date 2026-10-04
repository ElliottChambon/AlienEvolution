#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

#include "alien_evolution/development/RegulatoryDevelopment.hpp"
#include "alien_evolution/evaluation/Fitness.hpp"
#include "alien_evolution/evaluation/PhenotypeMetrics.hpp"
#include "alien_evolution/genetics/HeritableMutation.hpp"

namespace b6
{
    // Declared before stochastic evolution. No runtime parameter overrides.
    inline constexpr std::size_t replicates = 30, populationSize = 30, generations = 100;
    inline constexpr std::uint64_t baseSeed = 20261004, seedStride = 1009;
    inline constexpr double founderK = 1.0, probeLogDistance = 0.1, gradientTolerance = 1e-8;
    inline constexpr double tieTolerance = 1e-12;
    inline constexpr std::size_t landscapePoints = 41;
    inline constexpr double minimumK = 0.05, maximumK = 5.0;
    inline const std::vector<double> candidateResources{0.25, 0.5, 1.0, 1.5, 2.0};
    inline const std::vector<double> responseProbes{0.0, 0.25, 0.5, 1.0, 2.0};
    inline constexpr ae::RegulatoryDevelopmentSignals signalIds{100, 200};

    inline ae::RegulatoryDevelopmentConfig developmentConfig()
    {
        ae::RegulatoryDevelopmentConfig config{};
        config.depositionOutputNodeId = 3;
        config.neighborhoodLengthScale = 1.0;
        config.regulatoryTimeStep = 0.05;
        config.regulatoryStepsPerDevelopmentStep = 10;
        config.outputHalfSaturation = 0.2;
        config.outputCooperativity = 2.0;
        config.depositionRateScale = 0.25;
        return config;
    }

    inline ae::RegulatoryDevelopment development()
    {
        return ae::RegulatoryDevelopment(9, 9, 6, developmentConfig());
    }

    inline ae::HeritableProgram founder(double k = founderK)
    {
        return ae::HeritableProgram(ae::RegulatoryProgram({{3, 0.0, 0.01, 1.0}}, {}),
            ae::SensoryProgram({{signalIds.resourceSignalId, 3, 20.0, k, 2.0}}));
    }

    inline ae::HeritableMutationConfig mutationConfig(bool enabled)
    {
        ae::HeritableMutationConfig config{};
        config.sensory.rates.channelParameterPerChannel = enabled ? 0.10 : 0.0;
        config.sensory.quantitativeEffects.halfSaturationLogStdDev = enabled ? 0.15 : 0.0;
        return config;
    }

    inline ae::Environment environment(double resource)
    {
        ae::Environment result{};
        result.resourceAvailability = resource;
        return result;
    }

    inline ae::EnergeticsConfig energeticsConfig()
    {
        return {1.0, 0.10};
    }

    inline void checkFixedComponents(const ae::HeritableProgram& program)
    {
        const auto& regulatory = program.regulatoryProgram();
        const auto& sensory = program.sensoryProgram();
        if (regulatory.nodeCount() != 1 || regulatory.interactionCount() != 0
            || sensory.channelCount() != 1)
            throw std::logic_error("B6 fixed topology changed.");
        const auto& node = regulatory.nodes()[0];
        const auto& channel = sensory.channels()[0];
        if (node.id != 3 || node.initialActivity != 0.0 || node.basalProductionRate != 0.01
            || node.degradationRate != 1.0 || channel.signalId != signalIds.resourceSignalId
            || channel.targetNodeId != 3 || channel.foldChange != 20.0 || channel.cooperativity != 2.0)
            throw std::logic_error("B6 fixed regulation/wiring/response parameters changed.");
    }

    struct Evaluation
    {
        ae::Phenotype phenotype;
        ae::PhenotypeMetrics metrics;
        ae::EnergeticConsequences energy;
        double fitness;
    };

    inline Evaluation evaluate(const ae::HeritableProgram& program, double resource)
    {
        checkFixedComponents(program);
        const auto adapter = program.sensoryProgram().makeRegulatoryInputInterface(program.regulatoryProgram());
        auto phenotype = development().develop(program.regulatoryProgram(), adapter, signalIds, environment(resource));
        const auto metrics = ae::measurePhenotype(phenotype);
        const auto energy = ae::evaluateEnergetics(metrics, environment(resource), energeticsConfig());
        return {std::move(phenotype), metrics, energy, ae::calculateFitness(energy)};
    }

    struct Landscape
    {
        double resource, founderFitness, lowerFitness, higherFitness, gradient, optimumK, optimumFitness;
        bool boundary;
        int direction;
        std::vector<double> ks;
        std::vector<Evaluation> evaluations;
    };

    inline int gradientSign(double gradient)
    {
        return gradient > gradientTolerance ? 1 : (gradient < -gradientTolerance ? -1 : 0);
    }

    inline Landscape scan(double resource)
    {
        const double lower = evaluate(founder(founderK * std::exp(-probeLogDistance)), resource).fitness;
        const double higher = evaluate(founder(founderK * std::exp(probeLogDistance)), resource).fitness;
        Landscape result{resource, evaluate(founder(), resource).fitness, lower, higher,
            (higher - lower) / (2.0 * probeLogDistance), minimumK, -1.0, true, 0, {}, {}};
        result.direction = gradientSign(result.gradient);
        std::size_t best = 0;
        for (std::size_t i = 0; i < landscapePoints; ++i)
        {
            const double k = minimumK * std::pow(maximumK / minimumK,
                static_cast<double>(i) / static_cast<double>(landscapePoints - 1));
            result.ks.push_back(k);
            result.evaluations.push_back(evaluate(founder(k), resource));
            if (result.evaluations.back().fitness > result.optimumFitness)
            {
                best = i;
                result.optimumK = k;
                result.optimumFitness = result.evaluations.back().fitness;
            }
        }
        result.boundary = best == 0 || best + 1 == landscapePoints;
        return result;
    }

    // Strict comparisons give deterministic candidate-order tie breaking.
    inline std::vector<std::size_t> selectEnvironments(const std::vector<Landscape>& candidates)
    {
        std::vector<std::size_t> selected;
        for (std::size_t i = 0; i < candidates.size(); ++i)
        {
            if (candidates[i].founderFitness > 0.0 && candidates[i].direction != 0
                && (selected.empty() || std::abs(candidates[i].gradient) > std::abs(candidates[selected[0]].gradient)))
                selected = {i};
        }
        if (selected.empty()) return selected;
        std::size_t opposite = candidates.size();
        for (std::size_t i = 0; i < candidates.size(); ++i)
        {
            if (candidates[i].founderFitness > 0.0 && candidates[i].direction == -candidates[selected[0]].direction
                && (opposite == candidates.size() || std::abs(candidates[i].gradient) > std::abs(candidates[opposite].gradient)))
                opposite = i;
        }
        if (opposite != candidates.size()) selected.push_back(opposite);
        return selected;
    }

    inline double missing() { return std::numeric_limits<double>::quiet_NaN(); }

    inline double mean(const std::vector<double>& values)
    {
        if (values.empty()) return missing();
        double total = 0.0;
        for (double value : values) total += value;
        return total / static_cast<double>(values.size());
    }

    inline double sd(const std::vector<double>& values)
    {
        if (values.size() < 2) return 0.0;
        const double average = mean(values);
        double sum = 0.0;
        for (double value : values) sum += (value - average) * (value - average);
        return std::sqrt(sum / static_cast<double>(values.size() - 1));
    }

    inline double quantile(std::vector<double> values, double probability)
    {
        if (values.empty()) return missing();
        std::sort(values.begin(), values.end());
        const double position = probability * static_cast<double>(values.size() - 1);
        const auto lower = static_cast<std::size_t>(position);
        const auto upper = std::min(lower + 1, values.size() - 1);
        return values[lower] + (position - static_cast<double>(lower)) * (values[upper] - values[lower]);
    }

    inline double adaptiveShift(int direction, double meanLogK)
    {
        return direction * (meanLogK - std::log(founderK));
    }
} // namespace b6
