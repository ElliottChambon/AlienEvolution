#include "alien_evolution/physics/AdaptiveCertifiedPhysics.hpp"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <utility>

namespace
{
    void requireText(const std::string_view text, const char* message)
    {
        if (text.find_first_not_of(" \t\n\r\f\v") == std::string_view::npos)
            throw std::invalid_argument(message);
    }

    template<class Descriptor>
    void validateCategory(const std::vector<Descriptor>& descriptors)
    {
        std::set<std::string_view> keys;
        for (const auto& descriptor : descriptors)
        {
            descriptor.validate();
            if (!keys.insert(descriptor.key).second)
                throw std::invalid_argument("Duplicate descriptor key within contract category.");
        }
    }

    bool containsQuantity(const ae::AdaptivePhysicsContract& contract, const std::string_view key)
    {
        return std::any_of(contract.quantitiesOfInterest.begin(), contract.quantitiesOfInterest.end(),
            [key](const auto& quantity) { return quantity.key == key; });
    }

    void validateKeys(const std::vector<std::string>& keys)
    {
        for (const auto& key : keys) requireText(key, "Blank quantity-of-interest reference.");
    }

    void validateKeys(const std::vector<std::string>& keys, const ae::AdaptivePhysicsContract& contract)
    {
        contract.validate();
        for (const auto& key : keys)
            if (!containsQuantity(contract, key))
                throw std::invalid_argument("Undeclared quantity-of-interest reference.");
    }

    void validateReasons(const std::vector<ae::FidelityAuditReason>& reasons)
    {
        if (reasons.empty()) throw std::invalid_argument("Review/challenge requires at least one reason.");
        for (const auto reason : reasons)
        {
            switch (reason)
            {
            case ae::FidelityAuditReason::DeclaredValidityConcern:
            case ae::FidelityAuditReason::NovelRegimeOrState:
            case ae::FidelityAuditReason::ElitePhenotypeAudit:
            case ae::FidelityAuditReason::SuddenGainAudit:
            case ae::FidelityAuditReason::RandomShadowAudit:
            case ae::FidelityAuditReason::CrossFidelityDisagreement:
            case ae::FidelityAuditReason::ScientistRequestedReview: break;
            default: throw std::invalid_argument("Unknown fidelity audit reason.");
            }
        }
    }

    void validateRefs(const std::vector<ae::ScientificModelRef>& refs)
    {
        for (const auto& ref : refs) ref.validate();
    }
}

namespace ae
{
    ScientificModelRef::ScientificModelRef(std::string identifier, std::string version)
        : identifier_(std::move(identifier)), version_(std::move(version))
    {
        validate();
    }

    const std::string& ScientificModelRef::identifier() const { return identifier_; }
    const std::string& ScientificModelRef::version() const { return version_; }

    void ScientificModelRef::validate() const
    {
        requireText(identifier_, "Model reference requires an identifier.");
        requireText(version_, "Model reference requires a version.");
    }

    void ValidityCriterionDescriptor::validate() const
    {
        requireText(key, "Validity criterion requires a key.");
        requireText(criterionIdentifier, "Validity criterion requires a model-owned identifier.");
        requireText(description, "Validity criterion requires a description/scope.");
    }

    void ErrorMeasureDescriptor::validate() const
    {
        requireText(key, "Error measure requires a key.");
        requireText(quantityOfInterestKey, "Error measure requires a QoI key.");
        requireText(measureIdentifier, "Error measure requires a model-owned identifier.");
        requireText(unit, "Error measure requires a unit string.");
    }

    void AdaptivePhysicsContract::validate() const
    {
        model.validate();
        if (quantitiesOfInterest.empty()) throw std::invalid_argument("Contract requires at least one QoI.");
        validateCategory(quantitiesOfInterest);
        validateCategory(validityCriteria);
        validateCategory(errorMeasures);
        for (const auto& measure : errorMeasures)
            if (!containsQuantity(*this, measure.quantityOfInterestKey))
                throw std::invalid_argument("Error measure refers to an undeclared QoI.");
        validateRefs(referenceModels);
    }

    void FidelityReviewRequest::validate() const
    {
        requireText(identifier, "Review request requires an identifier.");
        currentModel.validate();
        validateRefs(candidateModels);
        validateKeys(quantityOfInterestKeys);
        validateReasons(reasons);
        if (contextReference) requireText(*contextReference, "Present review context requires a reference.");
    }

    void FidelityReviewRequest::validate(const AdaptivePhysicsContract& contract) const
    {
        validate();
        validateKeys(quantityOfInterestKeys, contract);
    }

    void PhysicsChallengeCase::validate() const
    {
        requireText(identifier, "Challenge case requires an identifier.");
        model.validate();
        validateKeys(quantityOfInterestKeys);
        validateReasons(reasons);
        validateRefs(referenceModels);
        requireText(contextReference, "Challenge case requires a reproduction/context reference.");
        requireText(description, "Challenge case requires a description.");
    }

    void PhysicsChallengeCase::validate(const AdaptivePhysicsContract& contract) const
    {
        validate();
        validateKeys(quantityOfInterestKeys, contract);
    }

    void PhysicsChallengeRegistry::add(const PhysicsChallengeCase& challenge)
    {
        challenge.validate();
        if (find(challenge.identifier)) throw std::invalid_argument("Duplicate challenge-case identifier.");
        cases_.push_back(challenge);
    }

    const std::vector<PhysicsChallengeCase>& PhysicsChallengeRegistry::cases() const { return cases_; }

    const PhysicsChallengeCase* PhysicsChallengeRegistry::find(const std::string_view identifier) const
    {
        const auto found = std::find_if(cases_.begin(), cases_.end(),
            [identifier](const auto& challenge) { return challenge.identifier == identifier; });
        return found == cases_.end() ? nullptr : &*found;
    }
} // namespace ae
