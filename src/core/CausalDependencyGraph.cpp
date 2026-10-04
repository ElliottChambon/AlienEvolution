#include "alien_evolution/core/CausalDependencyGraph.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace ae
{
    DerivedArtifactId::DerivedArtifactId(std::string value) : value_(std::move(value)) { validate(); }
    const std::string& DerivedArtifactId::value() const { return value_; }
    void DerivedArtifactId::validate() const
    {
        if (value_.find_first_not_of(" \t\n\r\f\v") == std::string::npos)
            throw std::invalid_argument("Dependency artifact ID must be nonblank.");
    }

    void CausalDependencyGraph::registerArtifact(const DependencyArtifact& artifact)
    {
        artifact.identifier.validate();
        if (find(artifact.identifier)) throw std::invalid_argument("Duplicate dependency artifact ID.");
        artifacts_.push_back(artifact);
    }

    void CausalDependencyGraph::addDependency(const DerivedArtifactId& upstream, const DerivedArtifactId& dependent)
    {
        if (!find(upstream) || !find(dependent)) throw std::invalid_argument("Unknown dependency endpoint.");
        for (const auto& edge : edges_)
            if (edge.upstream == upstream && edge.dependent == dependent)
                throw std::invalid_argument("Duplicate dependency edge.");
        edges_.push_back({upstream, dependent});
    }

    const std::vector<DependencyArtifact>& CausalDependencyGraph::artifacts() const { return artifacts_; }
    const std::vector<DependencyEdge>& CausalDependencyGraph::edges() const { return edges_; }

    const DependencyArtifact* CausalDependencyGraph::find(const DerivedArtifactId& id) const
    {
        const auto found = std::find_if(artifacts_.begin(), artifacts_.end(),
            [&id](const auto& artifact) { return artifact.identifier == id; });
        return found == artifacts_.end() ? nullptr : &*found;
    }

    std::vector<DerivedArtifactId> CausalDependencyGraph::directDependents(const DerivedArtifactId& id) const
    {
        if (!find(id)) throw std::invalid_argument("Unknown dependency query artifact.");
        std::vector<DerivedArtifactId> result;
        for (const auto& artifact : artifacts_)
            for (const auto& edge : edges_)
                if (edge.upstream == id && edge.dependent == artifact.identifier)
                    result.push_back(artifact.identifier);
        return result;
    }

    std::vector<DerivedArtifactId> CausalDependencyGraph::affectedArtifacts(const std::vector<DerivedArtifactId>& changed) const
    {
        for (const auto& id : changed)
            if (!find(id)) throw std::invalid_argument("Unknown changed dependency artifact.");
        auto pending = changed;
        std::vector<DerivedArtifactId> visited;
        while (!pending.empty())
        {
            auto current = pending.back(); pending.pop_back();
            if (std::find(visited.begin(), visited.end(), current) != visited.end()) continue;
            visited.push_back(current);
            for (const auto& edge : edges_)
                if (edge.upstream == current) pending.push_back(edge.dependent);
        }
        std::vector<DerivedArtifactId> result;
        for (const auto& artifact : artifacts_)
            if (std::find(visited.begin(), visited.end(), artifact.identifier) != visited.end())
                result.push_back(artifact.identifier);
        return result;
    }
} // namespace ae
