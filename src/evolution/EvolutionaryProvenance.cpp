#include "alien_evolution/evolution/EvolutionaryProvenance.hpp"

#include <algorithm>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace
{
    void requireText(std::string_view text)
    {
        if (text.find_first_not_of(" \t\n\r\f\v") == std::string_view::npos)
            throw std::invalid_argument("Provenance identifier must be nonblank.");
    }

    void validateIds(const std::vector<ae::ProvenanceEntityId>& ids)
    {
        for (std::size_t i = 0; i < ids.size(); ++i)
        {
            ids[i].validate();
            for (std::size_t j = 0; j < i; ++j)
                if (ids[i] == ids[j]) throw std::invalid_argument("Duplicate entity ID in event list.");
        }
    }
}

namespace ae
{
    ProvenanceEntityId::ProvenanceEntityId(std::string value) : value_(std::move(value)) { validate(); }
    const std::string& ProvenanceEntityId::value() const { return value_; }
    void ProvenanceEntityId::validate() const { requireText(value_); }

    ProvenanceEventId::ProvenanceEventId(std::string value) : value_(std::move(value)) { validate(); }
    const std::string& ProvenanceEventId::value() const { return value_; }
    void ProvenanceEventId::validate() const { requireText(value_); }

    void ProvenanceEvent::validate() const
    {
        identifier.validate();
        switch (kind)
        {
        case ProvenanceEventKind::Descent:
        case ProvenanceEventKind::Duplication:
        case ProvenanceEventKind::Recombination:
        case ProvenanceEventKind::Fusion:
        case ProvenanceEventKind::HorizontalAcquisition:
            if (parents.empty()) throw std::invalid_argument("Non-origin event requires a parent.");
            break;
        case ProvenanceEventKind::DeNovoOrigin: break;
        default: throw std::invalid_argument("Unknown provenance event kind.");
        }
        if (children.empty()) throw std::invalid_argument("Provenance event requires children.");
        validateIds(parents);
        validateIds(children);
        if (mechanismIdentifier) requireText(*mechanismIdentifier);
        if (contextReference) requireText(*contextReference);
    }

    void EvolutionaryProvenanceGraph::append(const ProvenanceEvent& event)
    {
        event.validate();
        if (findEvent(event.identifier)) throw std::invalid_argument("Duplicate provenance event ID.");
        for (const auto& parent : event.parents)
            if (!findEntity(parent)) throw std::invalid_argument("Unknown provenance parent ID.");
        for (const auto& child : event.children)
        {
            // Existing parents -> fresh children ensures forward-only ancestry.
            // A back edge/cycle must reuse an existing ID or a same-event parent;
            // reject both rather than editing historical identities.
            if (std::find(event.parents.begin(), event.parents.end(), child) != event.parents.end())
                throw std::invalid_argument("Provenance self-cycle.");
            if (findEntity(child)) throw std::invalid_argument("Child ID already exists; back edges are forbidden.");
        }
        // Stage both collections so even allocation/copy failure cannot leave
        // orphan entities or a partially recorded historical event.
        auto nextEvents = events_;
        auto nextEntities = entities_;
        nextEvents.push_back(event);
        for (const auto& child : event.children) nextEntities.push_back({child, event.identifier});
        events_.swap(nextEvents);
        entities_.swap(nextEntities);
    }

    const std::vector<ProvenanceEvent>& EvolutionaryProvenanceGraph::events() const { return events_; }
    const std::vector<ProvenanceEntity>& EvolutionaryProvenanceGraph::entities() const { return entities_; }

    const ProvenanceEvent* EvolutionaryProvenanceGraph::findEvent(const ProvenanceEventId& id) const
    {
        const auto found = std::find_if(events_.begin(), events_.end(),
            [&id](const auto& event) { return event.identifier == id; });
        return found == events_.end() ? nullptr : &*found;
    }

    const ProvenanceEntity* EvolutionaryProvenanceGraph::findEntity(const ProvenanceEntityId& id) const
    {
        const auto found = std::find_if(entities_.begin(), entities_.end(),
            [&id](const auto& entity) { return entity.identifier == id; });
        return found == entities_.end() ? nullptr : &*found;
    }

    std::vector<ProvenanceEntityId> EvolutionaryProvenanceGraph::parentsOf(const ProvenanceEntityId& id) const
    {
        const auto* entity = findEntity(id);
        if (!entity) throw std::invalid_argument("Unknown provenance query entity.");
        return findEvent(entity->originEvent)->parents;
    }

    std::vector<ProvenanceEntityId> EvolutionaryProvenanceGraph::childrenOf(const ProvenanceEntityId& id) const
    {
        if (!findEntity(id)) throw std::invalid_argument("Unknown provenance query entity.");
        std::vector<ProvenanceEntityId> result;
        for (const auto& event : events_)
            if (std::find(event.parents.begin(), event.parents.end(), id) != event.parents.end())
                result.insert(result.end(), event.children.begin(), event.children.end());
        return result;
    }

    bool EvolutionaryProvenanceGraph::isAncestor(const ProvenanceEntityId& ancestor, const ProvenanceEntityId& descendant) const
    {
        if (!findEntity(ancestor) || !findEntity(descendant))
            throw std::invalid_argument("Unknown ancestry query entity.");
        if (ancestor == descendant) return false;
        auto pending = parentsOf(descendant);
        std::vector<ProvenanceEntityId> visited;
        while (!pending.empty())
        {
            auto current = pending.back(); pending.pop_back();
            if (current == ancestor) return true;
            if (std::find(visited.begin(), visited.end(), current) != visited.end()) continue;
            visited.push_back(current);
            const auto parents = parentsOf(current);
            pending.insert(pending.end(), parents.begin(), parents.end());
        }
        return false;
    }
} // namespace ae
