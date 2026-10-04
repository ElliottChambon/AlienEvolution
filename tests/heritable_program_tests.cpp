#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include "alien_evolution/evolution/Reproduction.hpp"
#include "alien_evolution/genetics/HeritableProgram.hpp"

namespace
{
    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    template <typename Function>
    void requireInvalid(Function function)
    {
        try { function(); }
        catch (const std::invalid_argument&) { return; }
        throw std::runtime_error("Invalid sensory data/target accepted.");
    }

    bool sameChannels(const ae::SensoryProgram& a, const ae::SensoryProgram& b)
    {
        if (a.channelCount() != b.channelCount()) return false;
        for (std::size_t i = 0; i < a.channelCount(); ++i)
        {
            const auto& x = a.channels()[i];
            const auto& y = b.channels()[i];
            if (x.signalId != y.signalId || x.targetNodeId != y.targetNodeId
                || x.foldChange != y.foldChange || x.halfSaturation != y.halfSaturation
                || x.cooperativity != y.cooperativity) return false;
        }
        return true;
    }

    bool sameRegulation(const ae::RegulatoryProgram& a, const ae::RegulatoryProgram& b)
    {
        if (a.nodeCount() != b.nodeCount() || a.interactionCount() != b.interactionCount()) return false;
        for (std::size_t i = 0; i < a.nodeCount(); ++i)
        {
            const auto& x = a.nodes()[i];
            const auto& y = b.nodes()[i];
            if (x.id != y.id || x.initialActivity != y.initialActivity
                || x.basalProductionRate != y.basalProductionRate
                || x.degradationRate != y.degradationRate) return false;
        }
        for (std::size_t i = 0; i < a.interactionCount(); ++i)
        {
            const auto& x = a.interactions()[i];
            const auto& y = b.interactions()[i];
            if (x.sourceNodeId != y.sourceNodeId || x.targetNodeId != y.targetNodeId
                || x.foldChange != y.foldChange || x.halfSaturation != y.halfSaturation
                || x.cooperativity != y.cooperativity) return false;
        }
        return true;
    }
}

int main()
{
    try
    {
        const ae::RegulatoryProgram regulatory({{10, 0.1, 2.0, 1.0}, {20, 0.2, 3.0, 0.5}},
            {{10, 20, 2.0, 0.5, 2.0}});
        std::vector<ae::RegulatoryInputChannel> channels{
            {100, 10, 5.0, 2.0, 2.0}, {100, 20, 0.25, 1.0, 1.0},
            {100, 10, 3.0, 2.0, 1.0}
        };
        const ae::SensoryProgram sensory(channels);
        channels[0].foldChange = 999.0;
        require(sensory.channelCount() == 3 && sensory.channels()[0].foldChange == 5.0,
            "Sensory program did not own copied channel data.");
        require(sameChannels(sensory, ae::SensoryProgram(sensory.channels())),
            "Sensory storage changed fields/order.");
        const auto adapter = sensory.makeRegulatoryInputInterface(regulatory);
        const ae::RegulatoryInputInterface direct(regulatory, sensory.channels());
        for (double value : {0.0, 1.0, 2.0, 1e100})
        {
            for (std::uint64_t target : {10, 20})
            {
                require(adapter.modulationFactor(target, {{100, value}})
                    == direct.modulationFactor(target, {{100, value}}), "Adapter changed transduction.");
            }
        }
        require(adapter.modulationFactor(10, {{100, 2.0}}) == 6.0, "Shared channels lost.");
        require(ae::SensoryProgram{}.channelCount() == 0, "Default sensory program is not empty.");
        for (double invalid : {0.0, -1.0, std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()})
        {
            for (int parameter = 0; parameter < 3; ++parameter)
            {
                auto bad = sensory.channels();
                if (parameter == 0) bad[0].foldChange = invalid;
                if (parameter == 1) bad[0].halfSaturation = invalid;
                if (parameter == 2) bad[0].cooperativity = invalid;
                requireInvalid([&] { (void)ae::SensoryProgram(bad); });
            }
        }
        const ae::HeritableProgram heritable(regulatory, sensory);
        require(sameRegulation(heritable.regulatoryProgram(), regulatory)
            && sameChannels(heritable.sensoryProgram(), sensory), "Composition changed component data.");
        ae::Organism parent(heritable);
        require(&parent.regulatoryProgram() == &parent.heritableProgram().regulatoryProgram(),
            "Compatibility accessor did not delegate to owned component.");
        require(sameChannels(parent.heritableProgram().sensoryProgram(), sensory), "Organism lost sensory data.");
        require(!parent.hasFitness() && !parent.hasPhenotype(), "New organism was evaluated.");
        ae::Phenotype phenotype(2, 2);
        phenotype.setMaterial(0, 0, 1.0);
        parent.setPhenotype(phenotype);
        parent.setFitness(10.0);
        require(parent.fitness() == 10.0 && parent.phenotype().totalMaterial() == 1.0,
            "Evaluation behavior changed.");
        auto cleared = parent;
        cleared.clearEvaluation();
        require(!cleared.hasFitness() && !cleared.hasPhenotype()
            && sameChannels(cleared.heritableProgram().sensoryProgram(), sensory),
            "Clearing evaluation changed ownership.");
        ae::Organism compatibility(regulatory);
        require(compatibility.heritableProgram().sensoryProgram().channelCount() == 0
            && sameRegulation(compatibility.regulatoryProgram(), regulatory), "Regulatory-only construction changed.");
        compatibility.setFitness(10.0);

        ae::RegulatoryMutationGeneratorConfig mutation{};
        mutation.rates.nodeKineticPerNode = 5.0;
        mutation.rates.interactionParameterPerInteraction = 5.0;
        mutation.quantitativeEffects.nodeBasalProductionLogStdDev = 0.2;
        mutation.quantitativeEffects.nodeDegradationLogStdDev = 0.2;
        mutation.quantitativeEffects.interactionFoldChangeLogStdDev = 0.2;
        mutation.quantitativeEffects.interactionHalfSaturationLogStdDev = 0.2;
        mutation.quantitativeEffects.interactionCooperativityLogStdDev = 0.2;
        for (auto mode : {ae::SelectionMode::FitnessProportional, ae::SelectionMode::Uniform})
        {
            ae::Random random(1234), repeatRandom(1234), emptyRandom(1234), oracleRandom(1234);
            const auto offspring = ae::reproducePopulation(ae::Population({parent}), 8, random, mutation, mode);
            const auto repeat = ae::reproducePopulation(ae::Population({parent}), 8, repeatRandom, mutation, mode);
            const auto empty = ae::reproducePopulation(ae::Population({compatibility}), 8, emptyRandom, mutation, mode);
            bool changed = false;
            for (std::size_t i = 0; i < offspring.size(); ++i)
            {
                // Replay the pre-migration parent selection + regulatory
                // generator sequence, independently of heritable ownership.
                (void)oracleRandom.uniform(0.0, mode == ae::SelectionMode::Uniform ? 1.0 : 10.0);
                const auto expected = ae::generateRegulatoryOffspring(regulatory, mutation, oracleRandom);
                const auto& child = offspring.at(i);
                require(sameChannels(child.heritableProgram().sensoryProgram(), sensory), "Sensory inheritance changed.");
                require(sameRegulation(child.regulatoryProgram(), expected.offspringProgram)
                    && sameRegulation(child.regulatoryProgram(), repeat.at(i).regulatoryProgram())
                    && sameRegulation(child.regulatoryProgram(), empty.at(i).regulatoryProgram()),
                    "Regulatory mutation/RNG sequence or determinism changed.");
                require(sameChannels(child.heritableProgram().sensoryProgram(),
                    repeat.at(i).heritableProgram().sensoryProgram()), "Sensory inheritance is not deterministic.");
                require(!child.hasFitness() && !child.hasPhenotype(), "Offspring inherited evaluations.");
                changed |= !sameRegulation(child.regulatoryProgram(), regulatory);
            }
            require(changed, "Mutation comparison did not exercise regulatory changes.");
            require(random.raw() == oracleRandom.raw() && repeatRandom.raw() == emptyRandom.raw(),
                "Ownership migration consumed random draws.");
        }

        // No automatic repair: a structural loss leaves inherited sensory
        // targets unchanged and adapter validation later rejects the coupling.
        ae::RegulatoryMutationGeneratorConfig loss{};
        loss.rates.regulatoryNodeLossPerDeletableNode = 100.0;
        ae::Random lossRandom(1234);
        const auto lost = ae::reproducePopulation(ae::Population({parent}), 1, lossRandom, loss,
            ae::SelectionMode::FitnessProportional);
        require(lost.at(0).regulatoryProgram().nodeCount() == 1, "Node-loss setup failed.");
        require(sameChannels(lost.at(0).heritableProgram().sensoryProgram(), sensory),
            "Node loss repaired/deleted inherited channels.");
        requireInvalid([&] {
            (void)lost.at(0).heritableProgram().sensoryProgram().makeRegulatoryInputInterface(
                lost.at(0).regulatoryProgram());
        });
        const ae::SensoryProgram missing({{100, 999, 2.0, 1.0, 1.0}});
        const ae::HeritableProgram mismatched(regulatory, missing);
        requireInvalid([&] { (void)mismatched.sensoryProgram().makeRegulatoryInputInterface(regulatory); });
        require(sameChannels(mismatched.sensoryProgram(), missing), "Invalid coupling changed stored channels.");

        // Distinct parents must pass on the sensory component of the selected
        // parent, rather than a shared/default sensory container.
        const ae::RegulatoryProgram secondRegulatory({{30, 0.0, 4.0, 1.0}}, {});
        const ae::SensoryProgram secondSensory({{200, 30, 0.5, 3.0, 1.5}});
        ae::Organism second(ae::HeritableProgram(secondRegulatory, secondSensory));
        second.setFitness(10.0);
        ae::Random selectionRandom(4321);
        const auto mixed = ae::reproducePopulation(ae::Population({parent, second}), 32,
            selectionRandom, {}, ae::SelectionMode::Uniform);
        bool sawFirst = false, sawSecond = false;
        for (const auto& child : mixed.organisms())
        {
            const bool first = child.regulatoryProgram().containsNode(10);
            require(sameChannels(child.heritableProgram().sensoryProgram(), first ? sensory : secondSensory),
                "Sensory inheritance came from wrong parent.");
            sawFirst |= first;
            sawSecond |= !first;
        }
        require(sawFirst && sawSecond, "Parent-selection test did not exercise both lineages.");
        std::cout << "All heritable program tests passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
