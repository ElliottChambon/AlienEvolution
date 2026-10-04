#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "alien_evolution/physics/PhysicalCouplingProcess.hpp"

namespace ae
{
    // Immutable-style simulator metadata value: no field setters, lookup,
    // replacement or solver-selection behavior. Construction validates syntax.
    class ScientificModelRef
    {
    public:
        ScientificModelRef(std::string identifier, std::string version);
        [[nodiscard]] const std::string& identifier() const;
        [[nodiscard]] const std::string& version() const;
        void validate() const;
        bool operator==(const ScientificModelRef&) const = default;

    private:
        std::string identifier_;
        std::string version_;
    };

    using QuantityOfInterestDescriptor = PhysicalQuantityDescriptor;

    // Declarations only: no predicates, thresholds or domain criterion catalog.
    struct ValidityCriterionDescriptor
    {
        std::string key;
        std::string criterionIdentifier;
        std::string description;
        std::optional<std::string> note;
        void validate() const;
    };

    // Numerical/reduction error reporting vocabulary only. This is never a
    // scientific/model-form confidence score and performs no error calculation.
    struct ErrorMeasureDescriptor
    {
        std::string key;
        std::string quantityOfInterestKey;
        std::string measureIdentifier;
        std::string unit;
        std::optional<std::string> description;
        void validate() const;
    };

    // Simulator-owned quantity/regime-specific declarations, not certification.
    // Reference edges apply to the declared QoIs; use separate contracts for
    // different QoI scopes. Ordered references form a graph, never a fidelity
    // ladder or assertion of biological truth. A reference model may have no edges.
    struct AdaptivePhysicsContract
    {
        ScientificModelRef model;
        std::vector<QuantityOfInterestDescriptor> quantitiesOfInterest;
        std::vector<ValidityCriterionDescriptor> validityCriteria;
        std::vector<ErrorMeasureDescriptor> errorMeasures;
        std::vector<ScientificModelRef> referenceModels;
        std::optional<std::string> description;
        void validate() const;
    };

    // Reasons for future review when approximation uncertainty could change a
    // scientific/evolutionary conclusion; no policy threshold or automatic action.
    enum class FidelityAuditReason
    {
        DeclaredValidityConcern,
        NovelRegimeOrState,
        ElitePhenotypeAudit,
        SuddenGainAudit,
        RandomShadowAudit,
        CrossFidelityDisagreement,
        ScientistRequestedReview
    };

    struct FidelityReviewRequest
    {
        std::string identifier;
        ScientificModelRef currentModel;
        std::vector<ScientificModelRef> candidateModels;
        std::vector<std::string> quantityOfInterestKeys;
        std::vector<FidelityAuditReason> reasons;
        std::optional<std::string> contextReference;
        std::optional<std::string> note;

        void validate() const;
        // Checks declared key membership only, never evaluates validity criteria.
        void validate(const AdaptivePhysicsContract& contract) const;
    };

    // Validation artifact, independent of organism/heritable/physical state.
    // Context is an inert reproduction/snapshot reference, not a loaded snapshot.
    struct PhysicsChallengeCase
    {
        std::string identifier;
        ScientificModelRef model;
        std::vector<std::string> quantityOfInterestKeys;
        std::vector<FidelityAuditReason> reasons;
        std::vector<ScientificModelRef> referenceModels;
        std::string contextReference;
        std::string description;

        void validate() const;
        void validate(const AdaptivePhysicsContract& contract) const;
    };

    // Append-only for this in-memory registry's lifetime; no persistence or
    // runtime audit behavior. Existing entries cannot be edited or overwritten.
    class PhysicsChallengeRegistry
    {
    public:
        PhysicsChallengeRegistry() = default;
        PhysicsChallengeRegistry(const PhysicsChallengeRegistry&) = default;
        PhysicsChallengeRegistry& operator=(const PhysicsChallengeRegistry&) = delete;

        void add(const PhysicsChallengeCase& challenge);
        [[nodiscard]] const std::vector<PhysicsChallengeCase>& cases() const;
        // Returned pointers/element references may be invalidated by a later add.
        [[nodiscard]] const PhysicsChallengeCase* find(std::string_view identifier) const;

    private:
        std::vector<PhysicsChallengeCase> cases_;
    };
} // namespace ae
