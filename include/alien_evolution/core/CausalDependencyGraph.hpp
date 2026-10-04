#pragma once

#include <optional>
#include <string>
#include <vector>

namespace ae
{
    // Simulator artifact identity, not ancestry or inherited biological identity.
    class DerivedArtifactId
    {
    public:
        explicit DerivedArtifactId(std::string value);
        [[nodiscard]] const std::string& value() const;
        void validate() const;
        bool operator==(const DerivedArtifactId&) const = default;
    private:
        std::string value_;
    };

    struct DependencyArtifact
    {
        DerivedArtifactId identifier;
        std::optional<std::string> description;
    };

    struct DependencyEdge
    {
        DerivedArtifactId upstream;
        DerivedArtifactId dependent;
    };

    // Computational dependency bookkeeping only. Cycles (including self edges)
    // are permitted; this is not the acyclic historical provenance graph.
    // No recomputation, cache mutation, biological state changes or repair.
    class CausalDependencyGraph
    {
    public:
        void registerArtifact(const DependencyArtifact& artifact);
        void addDependency(const DerivedArtifactId& upstream, const DerivedArtifactId& dependent);
        [[nodiscard]] const std::vector<DependencyArtifact>& artifacts() const;
        [[nodiscard]] const std::vector<DependencyEdge>& edges() const;
        // Pointers/element references may be invalidated by registration.
        [[nodiscard]] const DependencyArtifact* find(const DerivedArtifactId& id) const;
        // Query results use artifact insertion order. Unknown IDs throw.
        [[nodiscard]] std::vector<DerivedArtifactId> directDependents(const DerivedArtifactId& id) const;
        // Includes changed roots and all transitively reachable dependents once,
        // regardless of input-root/edge order. Empty roots give an empty result.
        [[nodiscard]] std::vector<DerivedArtifactId> affectedArtifacts(const std::vector<DerivedArtifactId>& changed) const;
    private:
        std::vector<DependencyArtifact> artifacts_;
        std::vector<DependencyEdge> edges_;
    };
} // namespace ae
