#include "alien_evolution/evolution/EvolutionaryProvenance.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace
{
    using Entity = ae::ProvenanceEntityId;
    using Event = ae::ProvenanceEventId;
    using Kind = ae::ProvenanceEventKind;
    using Graph = ae::EvolutionaryProvenanceGraph;
    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }
    template<class Function> void rejects(Function function)
    {
        try { function(); }
        catch (const std::invalid_argument&) { return; }
        throw std::runtime_error("Malformed provenance accepted.");
    }
    static_assert(!std::is_convertible_v<Entity, Event> && !std::is_convertible_v<Event, Entity>);
    static_assert(!std::is_convertible_v<std::string, Entity> && !std::is_convertible_v<std::uint64_t, Entity>);
    static_assert(!std::is_copy_assignable_v<Graph>);
    static_assert(std::is_same_v<decltype(std::declval<Graph&>().events()), const std::vector<ae::ProvenanceEvent>&>);
    static_assert(std::is_same_v<decltype(std::declval<Graph&>().findEntity(std::declval<Entity>())), const ae::ProvenanceEntity*>);
    template<class T> concept HasRole = requires(T t) { t.functionalRole(); };
    template<class T> concept HasMutation = requires(T t) { t.mutate(); };
    template<class T> concept HasFitness = requires(T t) { t.setFitness(1.0); };
    static_assert(!HasRole<Graph> && !HasMutation<Graph> && !HasFitness<Graph>);
}

int main()
{
    try
    {
        for (const auto blank : {"", " \t\r\n"})
        {
            rejects([&] { Entity id{blank}; });
            rejects([&] { Event id{blank}; });
        }
        require(Entity{"exact ID"}.value() == "exact ID", "ID text changed.");
        constexpr std::array kinds{Kind::Descent, Kind::Duplication, Kind::Recombination,
            Kind::Fusion, Kind::HorizontalAcquisition, Kind::DeNovoOrigin};
        for (std::size_t i = 0; i < kinds.size(); ++i)
            for (std::size_t j = i + 1; j < kinds.size(); ++j)
                require(kinds[i] != kinds[j], "Historical event kinds are not distinct.");
        const Entity rootZ{"root-z"}, rootA{"root-a"}, descendant{"descendant"},
            siblingZ{"sibling-z"}, siblingA{"sibling-a"}, joined{"joined"}, fused{"fused"}, acquired{"acquired"};
        Graph graph;
        ae::ProvenanceEvent origin{Event{"origin"}, Kind::DeNovoOrigin, {}, {rootZ, rootA},
            "example.mechanism", "example.snapshot", "Root bookkeeping only"};
        graph.append(origin);
        origin.description = "Changed source";
        require(graph.findEvent(Event{"origin"})->description == "Root bookkeeping only", "Graph aliases source event.");
        require(graph.parentsOf(rootZ).empty(), "Root acquired fictitious ancestry.");
        graph.append({Event{"descent"}, Kind::Descent, {rootZ}, {descendant}, {}, {}, {}});
        graph.append({Event{"duplication"}, Kind::Duplication, {descendant}, {siblingZ, siblingA}, {}, {}, {}});
        graph.append({Event{"recombination"}, Kind::Recombination, {siblingA, rootA}, {joined}, {}, {}, {}});
        graph.append({Event{"fusion"}, Kind::Fusion, {joined, siblingZ}, {fused}, {}, {}, {}});
        graph.append({Event{"acquisition"}, Kind::HorizontalAcquisition, {rootA, fused}, {acquired}, {}, {}, {}});
        const std::vector<Entity> expected{rootZ, rootA, descendant, siblingZ, siblingA, joined, fused, acquired};
        require(graph.entities().size() == expected.size() && graph.events().size() == 6, "Historical counts changed.");
        for (std::size_t i = 0; i < expected.size(); ++i)
            require(graph.entities()[i].identifier == expected[i], "Entity insertion order changed.");
        const std::array eventNames{"origin", "descent", "duplication", "recombination", "fusion", "acquisition"};
        for (std::size_t i = 0; i < eventNames.size(); ++i)
            require(graph.events()[i].identifier == Event{eventNames[i]}, "Event insertion order changed.");
        require(graph.findEntity(siblingZ)->originEvent == Event{"duplication"}, "Origin event lookup changed.");
        require(graph.findEvent(Event{"origin"})->mechanismIdentifier == "example.mechanism" &&
            graph.findEvent(Event{"origin"})->contextReference == "example.snapshot", "Optional metadata lost.");
        require(graph.parentsOf(joined) == std::vector<Entity>{siblingA, rootA}, "Multi-parent order lost.");
        require(graph.childrenOf(descendant) == std::vector<Entity>{siblingZ, siblingA}, "Multi-child order lost.");
        require(graph.childrenOf(rootA) == std::vector<Entity>{joined, acquired}, "Children event order changed.");
        require(graph.childrenOf(acquired).empty(), "Leaf has children.");
        require(graph.isAncestor(rootZ, acquired) && graph.isAncestor(rootA, acquired) &&
            graph.isAncestor(descendant, siblingZ) && graph.isAncestor(descendant, siblingA), "Ancestry traversal failed.");
        require(!graph.isAncestor(acquired, rootZ) && !graph.isAncestor(rootZ, rootZ) &&
            !graph.isAncestor(siblingZ, siblingA) && !graph.isAncestor(siblingA, siblingZ) &&
            !graph.isAncestor(rootA, descendant), "False ancestry inferred.");
        require(!graph.findEntity(Entity{"missing"}) && !graph.findEvent(Event{"missing"}), "Missing lookup failed.");
        const auto eventCount = graph.events().size(); const auto entityCount = graph.entities().size();
        auto rejectedAppend = [&](const ae::ProvenanceEvent& event)
        {
            rejects([&] { graph.append(event); });
            require(graph.events().size() == eventCount && graph.entities().size() == entityCount,
                "Rejected append partially changed history.");
        };
        rejectedAppend({Event{"origin"}, Kind::DeNovoOrigin, {}, {Entity{"fresh"}}, {}, {}, {}});
        rejectedAppend({Event{"unknown-parent"}, Kind::Descent, {Entity{"missing"}}, {Entity{"fresh"}}, {}, {}, {}});
        rejectedAppend({Event{"existing-child"}, Kind::DeNovoOrigin, {}, {rootZ}, {}, {}, {}});
        rejectedAppend({Event{"cycle"}, Kind::Descent, {acquired}, {rootZ}, {}, {}, {}});
        rejectedAppend({Event{"self-cycle"}, Kind::Descent, {rootZ}, {rootZ}, {}, {}, {}});
        rejectedAppend({Event{"same-event-cycle"}, Kind::Descent, {Entity{"fresh"}}, {Entity{"fresh"}}, {}, {}, {}});
        rejectedAppend({Event{"duplicate-child"}, Kind::DeNovoOrigin, {}, {Entity{"fresh"}, Entity{"fresh"}}, {}, {}, {}});
        rejectedAppend({Event{"duplicate-parent"}, Kind::Descent, {rootZ, rootZ}, {Entity{"fresh"}}, {}, {}, {}});
        rejectedAppend({Event{"no-child"}, Kind::DeNovoOrigin, {}, {}, {}, {}, {}});
        rejectedAppend({Event{"no-parent"}, Kind::Descent, {}, {Entity{"fresh"}}, {}, {}, {}});
        rejectedAppend({Event{"unknown-kind"}, static_cast<Kind>(999), {}, {Entity{"fresh"}}, {}, {}, {}});
        rejectedAppend({Event{"blank-mechanism"}, Kind::DeNovoOrigin, {}, {Entity{"fresh"}}, " \t", {}, {}});
        rejectedAppend({Event{"blank-context"}, Kind::DeNovoOrigin, {}, {Entity{"fresh"}}, {}, "", {}});
        rejects([&] { (void)graph.parentsOf(Entity{"missing"}); });
        rejects([&] { (void)graph.childrenOf(Entity{"missing"}); });
        rejects([&] { (void)graph.isAncestor(rootZ, Entity{"missing"}); });
        rejects([&] { (void)graph.isAncestor(Entity{"missing"}, rootZ); });
        const auto copied = graph;
        graph.append({Event{"later"}, Kind::Descent, {acquired}, {Entity{"later"}}, {}, {}, {}});
        require(!copied.findEntity(Entity{"later"}) && copied.events().size() == eventCount, "Graph copy aliases mutable history.");
        // No mechanism-specific cardinality assumptions beyond safe structure.
        graph.append({Event{"neutral-shape"}, Kind::Duplication, {rootZ, rootA},
            {Entity{"extra-z"}, Entity{"extra-a"}}, {}, {}, {}});
        std::cout << "All evolutionary provenance tests passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
