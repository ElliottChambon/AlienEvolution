#include <bit>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "alien_evolution/evolution/Reproduction.hpp"
#include "alien_evolution/genetics/HeritableConstructionState.hpp"

namespace
{
    using HCS = ae::HeritableConstructionState;

    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    bool exact(double a, double b)
    {
        return std::bit_cast<std::uint64_t>(a) == std::bit_cast<std::uint64_t>(b);
    }

    void sameProgram(const ae::HeritableProgram& a, const ae::HeritableProgram& b)
    {
        const auto& x = a.regulatoryProgram(); const auto& y = b.regulatoryProgram();
        require(x.nodeCount() == y.nodeCount() && x.interactionCount() == y.interactionCount(),
            "Regulatory topology/count changed.");
        for (std::size_t i = 0; i < x.nodeCount(); ++i)
        {
            const auto& n = x.nodes()[i]; const auto& m = y.nodes()[i];
            require(n.id == m.id && exact(n.initialActivity, m.initialActivity) &&
                exact(n.basalProductionRate, m.basalProductionRate) && exact(n.degradationRate, m.degradationRate),
                "Node content/order changed.");
        }
        for (std::size_t i = 0; i < x.interactionCount(); ++i)
        {
            const auto& n = x.interactions()[i]; const auto& m = y.interactions()[i];
            require(n.sourceNodeId == m.sourceNodeId && n.targetNodeId == m.targetNodeId &&
                exact(n.foldChange, m.foldChange) && exact(n.halfSaturation, m.halfSaturation) &&
                exact(n.cooperativity, m.cooperativity), "Interaction content/order changed.");
        }
        const auto& s = a.sensoryProgram(); const auto& t = b.sensoryProgram();
        require(s.channelCount() == t.channelCount(), "Sensory count changed.");
        for (std::size_t i = 0; i < s.channelCount(); ++i)
        {
            const auto& n = s.channels()[i]; const auto& m = t.channels()[i];
            require(n.signalId == m.signalId && n.targetNodeId == m.targetNodeId &&
                exact(n.foldChange, m.foldChange) && exact(n.halfSaturation, m.halfSaturation) &&
                exact(n.cooperativity, m.cooperativity), "Sensory content/order changed.");
        }
    }

    ae::HeritableProgram fixture()
    {
        return ae::HeritableProgram(
            ae::RegulatoryProgram({{20, 0.2, 3.0, 0.5}, {10, 0.1, 2.0, 1.0}},
                {{20, 10, 0.25, 1.0, 1.5}, {10, 20, 2.0, 0.5, 2.0}}),
            ae::SensoryProgram({{200, 20, 0.25, 1.0, 1.0}, {100, 10, 5.0, 2.0, 2.0},
                {100, 10, 3.0, 2.0, 1.0}}));
    }

    void sameRandomContinuation(ae::Random& a, ae::Random& b)
    {
        // Check raw engine draws and normal draws to catch cached spare-normal
        // differences as well as extra random consumption in the compatibility seam.
        for (int i = 0; i < 16; ++i)
        {
            require(a.raw() == b.raw(), "Raw RNG continuation changed.");
            require(exact(a.normal(0.0, 1.0), b.normal(0.0, 1.0)), "Normal RNG continuation changed.");
        }
    }

    // Replay the pre-M4 selection -> regulatory generator -> sensory generator
    // sequence without using Organism/HCS access or generateHeritableOffspring.
    // Test fixtures have strictly positive fitness, avoiding defensive fallbacks.
    void checkReproduction(const ae::HeritableMutationConfig& config, ae::SelectionMode mode,
        bool regulatoryOnly)
    {
        auto first = fixture();
        const ae::HeritableProgram second(
            ae::RegulatoryProgram({{30, 0.3, 4.0, 1.5}}, {}),
            ae::SensoryProgram({{300, 30, 0.5, 3.0, 1.5}}));
        if (regulatoryOnly) first = ae::HeritableProgram(first.regulatoryProgram());
        const auto secondPayload = regulatoryOnly ? ae::HeritableProgram(second.regulatoryProgram()) : second;
        ae::Organism parentA(HCS::fromPrototypeHeritableProgram(first));
        ae::Organism parentB(secondPayload);
        parentA.setFitness(3.0); parentB.setFitness(7.0);
        ae::Random actual(9876), baseline(9876);
        const ae::Population parents({parentA, parentB});
        const auto offspring = regulatoryOnly
            ? ae::reproducePopulation(parents, 24, actual, config.regulatory, mode)
            : ae::reproducePopulation(parents, 24, config, actual, mode);
        bool sawFirst = false, sawSecond = false;
        for (std::size_t i = 0; i < offspring.size(); ++i)
        {
            const auto target = baseline.uniform(0.0, mode == ae::SelectionMode::Uniform ? 2.0 : 10.0);
            const bool selectedFirst = target < (mode == ae::SelectionMode::Uniform ? 1.0 : 3.0);
            const auto& payload = selectedFirst ? first : secondPayload;
            auto regulation = ae::generateRegulatoryOffspring(payload.regulatoryProgram(), config.regulatory, baseline);
            auto sensing = ae::generateSensoryOffspring(payload.sensoryProgram(),
                regulatoryOnly ? ae::SensoryMutationGeneratorConfig{} : config.sensory, baseline);
            const ae::HeritableProgram expected(std::move(regulation.offspringProgram), std::move(sensing.offspringProgram));
            const auto& child = offspring.at(i);
            sameProgram(child.heritableProgram(), expected);
            require(&child.heritableProgram() == &child.constructionState().prototypeHeritableProgram(),
                "Reproduction created parallel hereditary state.");
            require(!child.hasFitness() && !child.hasPhenotype(), "Offspring inherited evaluation.");
            sawFirst |= selectedFirst; sawSecond |= !selectedFirst;
        }
        require(offspring.size() == 24 && sawFirst && sawSecond, "Selection fixture missed a parent.");
        sameRandomContinuation(actual, baseline);
        sameProgram(parents.at(0).heritableProgram(), first);
        sameProgram(parents.at(1).heritableProgram(), secondPayload);
    }

    template<class T> concept HasAncestry = requires(T t) { t.ancestorId(); };
    template<class T> concept HasProvenance = requires(T t) { t.provenance(); };
    template<class T> concept HasRole = requires(T t) { t.functionalRole(); };
    template<class T> concept HasMutation = requires(T t) { t.mutate(); };
    template<class T> concept AcceptsPrototypeMutation = requires(T t, ae::Random r)
    {
        ae::generateHeritableOffspring(t, ae::HeritableMutationConfig{}, r);
    };
    static_assert(!HasAncestry<HCS> && !HasProvenance<HCS> && !HasRole<HCS> && !HasMutation<HCS>);
    static_assert(!AcceptsPrototypeMutation<HCS>);
    static_assert(!std::is_convertible_v<HCS, ae::HeritableProgram>);
    static_assert(std::is_same_v<decltype(std::declval<HCS&>().prototypeHeritableProgram()), const ae::HeritableProgram&>);
}

int main()
{
    try
    {
        auto source = fixture();
        const auto expected = source;
        auto state = HCS::fromPrototypeHeritableProgram(source);
        sameProgram(state.prototypeHeritableProgram(), expected);
        const ae::HeritableProgram roundTrip = state.prototypeHeritableProgram();
        sameProgram(roundTrip, expected);
        require(&state.prototypeHeritableProgram() != &source && &roundTrip != &state.prototypeHeritableProgram(),
            "Compatibility payload did not own its value.");
        require(state.prototypeHeritableProgram().regulatoryProgram().nodes().data() != source.regulatoryProgram().nodes().data() &&
            state.prototypeHeritableProgram().sensoryProgram().channels().data() != source.sensoryProgram().channels().data(),
            "HCS aliases source component storage.");
        source = ae::HeritableProgram(ae::RegulatoryProgram({{999, 0.0, 1.0, 1.0}}, {}));
        sameProgram(state.prototypeHeritableProgram(), expected);
        auto temporary = HCS::fromPrototypeHeritableProgram(fixture());
        sameProgram(temporary.prototypeHeritableProgram(), expected);
        auto copied = state;
        require(copied.prototypeHeritableProgram().regulatoryProgram().nodes().data() !=
            state.prototypeHeritableProgram().regulatoryProgram().nodes().data(), "HCS copy aliases storage.");
        auto moved = std::move(copied);
        sameProgram(moved.prototypeHeritableProgram(), expected);
        auto assigned = HCS::fromPrototypeHeritableProgram(source);
        assigned = state;
        sameProgram(assigned.prototypeHeritableProgram(), expected);
        auto moveAssigned = HCS::fromPrototypeHeritableProgram(source);
        moveAssigned = std::move(assigned);
        sameProgram(moveAssigned.prototypeHeritableProgram(), expected);
        moved = HCS::fromPrototypeHeritableProgram(source);
        sameProgram(state.prototypeHeritableProgram(), expected);

        ae::Organism organism(state), legacy(expected), regulatoryOnly(expected.regulatoryProgram());
        sameProgram(organism.constructionState().prototypeHeritableProgram(), expected);
        sameProgram(legacy.heritableProgram(), expected);
        sameProgram(regulatoryOnly.heritableProgram(), ae::HeritableProgram(expected.regulatoryProgram()));
        require(&organism.heritableProgram() == &organism.constructionState().prototypeHeritableProgram() &&
            &organism.regulatoryProgram() == &organism.heritableProgram().regulatoryProgram() &&
            &legacy.heritableProgram() == &legacy.constructionState().prototypeHeritableProgram(),
            "Compatibility accessors did not delegate to one HCS payload.");
        require(!organism.hasFitness() && !organism.hasPhenotype(), "New HCS organism was evaluated.");
        const auto* payloadAddress = &organism.heritableProgram();
        ae::Phenotype phenotype(2, 2); phenotype.setMaterial(0, 0, 1.0);
        organism.setPhenotype(phenotype); organism.setFitness(10.0);
        require(organism.fitness() == 10.0 && organism.phenotype().totalMaterial() == 1.0, "Evaluation storage changed.");
        sameProgram(organism.heritableProgram(), expected);
        organism.clearEvaluation();
        require(!organism.hasFitness() && !organism.hasPhenotype() && payloadAddress == &organism.heritableProgram(),
            "Clearing evaluation changed HCS ownership.");
        sameProgram(organism.heritableProgram(), expected);

        ae::HeritableMutationConfig config;
        config.regulatory.rates.nodeKineticPerNode = 5.0;
        config.regulatory.rates.interactionParameterPerInteraction = 5.0;
        config.regulatory.quantitativeEffects = {0.2, 0.2, 0.2, 0.2, 0.2};
        config.sensory.rates.channelParameterPerChannel = 10.0;
        config.sensory.quantitativeEffects = {0.1, 0.2, 0.3};
        for (const auto mode : {ae::SelectionMode::Uniform, ae::SelectionMode::FitnessProportional})
        {
            checkReproduction(config, mode, true);
            checkReproduction(config, mode, false);
            checkReproduction({}, mode, false);
        }
        ae::Random joint(5678), baseline(5678);
        const auto result = ae::generateHeritableOffspring(state.prototypeHeritableProgram(), config, joint);
        auto regulatory = ae::generateRegulatoryOffspring(expected.regulatoryProgram(), config.regulatory, baseline);
        auto sensory = ae::generateSensoryOffspring(expected.sensoryProgram(), config.sensory, baseline);
        sameProgram(result.offspringProgram,
            ae::HeritableProgram(std::move(regulatory.offspringProgram), std::move(sensory.offspringProgram)));
        require(!result.regulatoryEvents.empty() && !result.sensoryEvents.empty(), "Joint fixture missed mutation.");
        sameRandomContinuation(joint, baseline);

        ae::RegulatoryMutationGeneratorConfig loss;
        loss.rates.regulatoryNodeLossPerDeletableNode = 100.0;
        ae::Random lossRandom(1234);
        legacy.setFitness(10.0);
        const auto lost = ae::reproducePopulation(ae::Population({legacy}), 1, lossRandom, loss,
            ae::SelectionMode::FitnessProportional);
        const auto& child = lost.at(0);
        require(child.regulatoryProgram().nodeCount() == 1, "Structural loss fixture failed.");
        sameProgram(child.heritableProgram(), ae::HeritableProgram(child.regulatoryProgram(), expected.sensoryProgram()));
        bool rejected = false;
        try { (void)child.heritableProgram().sensoryProgram().makeRegulatoryInputInterface(child.regulatoryProgram()); }
        catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "Missing sensory target was repaired or accepted.");
        // HCS construction itself adds no cross-component validation or repair.
        const ae::HeritableProgram mismatched(expected.regulatoryProgram(), ae::SensoryProgram({{1, 999, 2.0, 1.0, 1.0}}));
        const ae::Organism unresolved(HCS::fromPrototypeHeritableProgram(mismatched));
        sameProgram(unresolved.heritableProgram(), mismatched);
        std::cout << "All heritable construction state compatibility tests passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
