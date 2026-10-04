#include "alien_evolution/core/CausalDependencyGraph.hpp"
#include "alien_evolution/evolution/EvolutionaryProvenance.hpp"
#include "alien_evolution/evolution/Organism.hpp"

#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace
{
    using Id = ae::DerivedArtifactId;
    using Graph = ae::CausalDependencyGraph;
    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }
    template<class Function> void rejects(Function function)
    {
        try { function(); }
        catch (const std::invalid_argument&) { return; }
        throw std::runtime_error("Malformed dependency graph accepted.");
    }
    static_assert(!std::is_convertible_v<Id, ae::ProvenanceEntityId> && !std::is_convertible_v<ae::ProvenanceEntityId, Id>);
    static_assert(!std::is_convertible_v<Id, ae::ProvenanceEventId> && !std::is_convertible_v<ae::ProvenanceEventId, Id>);
    static_assert(!std::is_convertible_v<std::string, Id> && !std::is_convertible_v<std::uint64_t, Id>);
    static_assert(std::is_same_v<decltype(std::declval<Graph&>().artifacts()), const std::vector<ae::DependencyArtifact>&>);
    static_assert(std::is_same_v<decltype(std::declval<Graph&>().edges()), const std::vector<ae::DependencyEdge>&>);
}

int main()
{
    try
    {
        for (const auto blank : {"", " \t\r\n"}) rejects([&] { Id bad{blank}; });
        require(Id{"exact ID"}.value() == "exact ID", "Artifact ID changed.");
        const Id leaf{"leaf"}, rootZ{"root-z"}, branchA{"branch-a"}, rootA{"root-a"},
            branchZ{"branch-z"}, isolated{"isolated"};
        Graph graph;
        const std::vector<Id> order{leaf, rootZ, branchA, rootA, branchZ, isolated};
        for (const auto& id : order) graph.registerArtifact({id, "Neutral fixture"});
        for (std::size_t i = 0; i < order.size(); ++i)
            require(graph.artifacts()[i].identifier == order[i], "Artifact insertion order changed.");
        require(graph.find(rootZ)->description == "Neutral fixture" && !graph.find(Id{"missing"}), "Artifact lookup changed.");
        graph.addDependency(rootZ, branchZ);
        graph.addDependency(rootZ, branchA);
        graph.addDependency(branchA, leaf);
        graph.addDependency(branchZ, leaf); // Diamond: leaf must occur only once.
        graph.addDependency(rootA, branchZ);
        require(graph.edges()[0].upstream == rootZ && graph.edges()[0].dependent == branchZ,
            "Edge direction/order changed.");
        require(graph.directDependents(rootZ) == std::vector<Id>{branchA, branchZ}, "Direct-dependent order changed.");
        require(graph.directDependents(leaf).empty(), "Dependency edges treated as undirected.");
        const std::vector<Id> fromZ{leaf, rootZ, branchA, branchZ};
        require(graph.affectedArtifacts({rootZ}) == fromZ && graph.affectedArtifacts({rootZ}) == fromZ,
            "Diamond invalidation closure/determinism failed.");
        require(graph.affectedArtifacts({rootA}) == std::vector<Id>{leaf, rootA, branchZ}, "Reverse dependency inferred.");
        const std::vector<Id> unionOrder{leaf, rootZ, branchA, rootA, branchZ};
        require(graph.affectedArtifacts({rootA, rootZ, rootZ}) == unionOrder &&
            graph.affectedArtifacts({rootZ, rootA}) == unionOrder, "Multi-root union/order changed.");
        require(graph.affectedArtifacts({}).empty() && graph.affectedArtifacts({isolated}) == std::vector<Id>{isolated},
            "Empty/isolated closure failed.");
        const auto nodesBefore = graph.artifacts().size(); const auto edgesBefore = graph.edges().size();
        rejects([&] { graph.registerArtifact({rootZ, "Must not replace"}); });
        rejects([&] { graph.addDependency(rootZ, branchZ); });
        rejects([&] { graph.addDependency(Id{"missing"}, branchA); });
        rejects([&] { graph.addDependency(branchA, Id{"missing"}); });
        rejects([&] { (void)graph.directDependents(Id{"missing"}); });
        rejects([&] { (void)graph.affectedArtifacts({rootZ, Id{"missing"}}); });
        require(graph.artifacts().size() == nodesBefore && graph.edges().size() == edgesBefore &&
            graph.find(rootZ)->description == "Neutral fixture", "Rejected operation changed graph.");
        graph.addDependency(leaf, rootZ); // Computational cycle is allowed.
        graph.addDependency(isolated, isolated); // Self cycle is also safe.
        require(graph.affectedArtifacts({rootZ}) == fromZ, "Cyclic traversal missed/duplicated artifacts.");
        require(graph.affectedArtifacts({leaf}) == fromZ, "Cycle reachability failed.");
        require(graph.affectedArtifacts({isolated}) == std::vector<Id>{isolated}, "Self-cycle traversal failed.");
        rejects([&] { graph.addDependency(isolated, isolated); });
        auto copied = graph;
        graph.registerArtifact({Id{"later"}, std::nullopt});
        require(!copied.find(Id{"later"}), "Graph copy aliases mutable artifacts.");

        // Graphs are independent of biological state: queries never clear
        // evaluation, mutate payloads, or promote prototype IDs to ancestry IDs.
        ae::Organism organism(ae::HeritableProgram(ae::RegulatoryProgram({{10, 0.1, 2.0, 1.0}}, {}),
            ae::SensoryProgram({{100, 10, 5.0, 2.0, 2.0}})));
        ae::Phenotype phenotype(2, 2); phenotype.setMaterial(0, 0, 1.0);
        organism.setPhenotype(phenotype); organism.setFitness(7.0);
        const auto* payload = &organism.heritableProgram();
        ae::EvolutionaryProvenanceGraph history;
        history.append({ae::ProvenanceEventId{"fixture-event"}, ae::ProvenanceEventKind::DeNovoOrigin,
            {}, {ae::ProvenanceEntityId{"fixture-entity"}}, {}, {}, {}});
        const auto affected = graph.affectedArtifacts({rootZ, rootA});
        require(affected == unionOrder && organism.hasFitness() && organism.fitness() == 7.0 &&
            organism.hasPhenotype() && organism.phenotype().totalMaterial() == 1.0 &&
            payload == &organism.constructionState().prototypeHeritableProgram() &&
            organism.regulatoryProgram().nodes()[0].id == 10 &&
            organism.regulatoryProgram().nodes()[0].basalProductionRate == 2.0 &&
            organism.heritableProgram().sensoryProgram().channels()[0].targetNodeId == 10 &&
            organism.heritableProgram().sensoryProgram().channels()[0].foldChange == 5.0,
            "Bookkeeping query changed organism/program/evaluation state.");
        std::cout << "All causal dependency graph tests passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
