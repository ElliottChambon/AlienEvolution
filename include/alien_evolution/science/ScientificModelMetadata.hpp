#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ae
{
    // Nominal categories, never a confidence scale. Underlying enum values
    // have no scientific meaning; only evidenceCode supplies stable codes.
    enum class EvidenceStatus
    {
        EstablishedPhysicalInteraction,
        DemonstratedBiologicalSensoryUse,
        DemonstratedBiologicalResponse,
        UnresolvedMechanism,
        MechanisticallyGroundedExtrapolation,
        Speculative
    };

    [[nodiscard]] std::string_view evidenceCode(EvidenceStatus status);
    [[nodiscard]] std::optional<EvidenceStatus> evidenceStatusFromCode(std::string_view code);

    // Prevent accidental interpretation of declaration order as confidence.
    bool operator<(EvidenceStatus, EvidenceStatus) = delete;
    bool operator<=(EvidenceStatus, EvidenceStatus) = delete;
    bool operator>(EvidenceStatus, EvidenceStatus) = delete;
    bool operator>=(EvidenceStatus, EvidenceStatus) = delete;

    // Intrinsic physical stochasticity belongs to later physical models,
    // not to either of these metadata uncertainty categories.
    enum class UncertaintyKind
    {
        NumericalReduction,
        ScientificModelForm
    };

    struct LiteratureReference
    {
        std::string label;
        std::string source;
        std::optional<std::string> note;
        void validate() const;
    };

    struct EvidenceRecord
    {
        EvidenceStatus status;
        std::string claim;
        std::vector<LiteratureReference> references;
        std::optional<std::string> limitation;
        void validate() const;
    };

    struct ParameterMetadata
    {
        std::string key;
        std::string unit;
        std::optional<std::string> description;
        void validate() const;
    };

    // Simulator/scientific metadata, never inherited organism state.
    // Collections retain insertion order. Relationships are inert identifiers;
    // they neither resolve references nor select/replace models.
    struct ScientificModelMetadata
    {
        std::string identifier;
        std::string name;
        std::string version;
        std::optional<std::string> description;
        std::vector<std::string> supersedes;
        std::vector<std::string> alternatives;
        std::vector<EvidenceRecord> evidence;
        std::vector<ParameterMetadata> parameters;
        std::vector<std::string> assumptions;
        std::string validityScope;
        std::vector<UncertaintyKind> uncertainties;

        // Explicit, read-only syntactic validation; no scientific certification.
        // Call after assembling/editing these plain metadata records.
        void validate() const;
    };
} // namespace ae
