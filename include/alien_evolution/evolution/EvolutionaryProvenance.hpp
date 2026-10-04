#pragma once

#include <optional>
#include <string>
#include <vector>

namespace ae
{
    // Simulator bookkeeping identities, unrelated to prototype program IDs.
    class ProvenanceEntityId
    {
    public:
        explicit ProvenanceEntityId(std::string value);
        [[nodiscard]] const std::string& value() const;
        void validate() const;
        bool operator==(const ProvenanceEntityId&) const = default;
    private:
        std::string value_;
    };

    class ProvenanceEventId
    {
    public:
        explicit ProvenanceEventId(std::string value);
        [[nodiscard]] const std::string& value() const;
        void validate() const;
        bool operator==(const ProvenanceEventId&) const = default;
    private:
        std::string value_;
    };

    // Historical relationship vocabulary only, never mutation operators.
    enum class ProvenanceEventKind
    {
        Descent,
        Duplication,
        Recombination,
        Fusion,
        HorizontalAcquisition,
        DeNovoOrigin
    };

    struct ProvenanceEvent
    {
        ProvenanceEventId identifier;
        ProvenanceEventKind kind;
        std::vector<ProvenanceEntityId> parents;
        std::vector<ProvenanceEntityId> children;
        std::optional<std::string> mechanismIdentifier;
        std::optional<std::string> contextReference;
        std::optional<std::string> description;
        void validate() const;
    };

    struct ProvenanceEntity
    {
        ProvenanceEntityId identifier;
        ProvenanceEventId originEvent;
    };

    // Append-only immutable history for this graph's lifetime, with zero physical
    // influence. Roots are recorded by parentless DeNovoOrigin events, not by
    // inventing earlier ancestry. No cardinality rules beyond nonempty children
    // and requiring parents for non-origin events; no mechanism is certified.
    class EvolutionaryProvenanceGraph
    {
    public:
        EvolutionaryProvenanceGraph() = default;
        EvolutionaryProvenanceGraph(const EvolutionaryProvenanceGraph&) = default;
        EvolutionaryProvenanceGraph& operator=(const EvolutionaryProvenanceGraph&) = delete;

        void append(const ProvenanceEvent& event);
        [[nodiscard]] const std::vector<ProvenanceEvent>& events() const;
        [[nodiscard]] const std::vector<ProvenanceEntity>& entities() const;
        // Pointers/element references may be invalidated by subsequent append.
        [[nodiscard]] const ProvenanceEvent* findEvent(const ProvenanceEventId& id) const;
        [[nodiscard]] const ProvenanceEntity* findEntity(const ProvenanceEntityId& id) const;
        // Unknown query IDs throw. Parents retain event-list order; children
        // retain event insertion order then each event's child-list order.
        [[nodiscard]] std::vector<ProvenanceEntityId> parentsOf(const ProvenanceEntityId& id) const;
        [[nodiscard]] std::vector<ProvenanceEntityId> childrenOf(const ProvenanceEntityId& id) const;
        // Strict ancestry: an entity is not its own ancestor.
        [[nodiscard]] bool isAncestor(const ProvenanceEntityId& ancestor, const ProvenanceEntityId& descendant) const;
    private:
        std::vector<ProvenanceEvent> events_;
        std::vector<ProvenanceEntity> entities_;
    };
} // namespace ae
