#include "alien_evolution/physics/AdaptiveCertifiedPhysics.hpp"

#include <array>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace
{
    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    template<class Function> void rejects(Function function)
    {
        try { function(); }
        catch (const std::invalid_argument&) { return; }
        throw std::runtime_error("Malformed contract metadata was accepted.");
    }

    using Ref = ae::ScientificModelRef;
    using Contract = ae::AdaptivePhysicsContract;
    using Reason = ae::FidelityAuditReason;
    constexpr std::array reasons{Reason::DeclaredValidityConcern, Reason::NovelRegimeOrState,
        Reason::ElitePhenotypeAudit, Reason::SuddenGainAudit, Reason::RandomShadowAudit,
        Reason::CrossFidelityDisagreement, Reason::ScientistRequestedReview};

    template<class T> concept HasStep = requires(T t) { t.step(); };
    template<class T> concept HasSolve = requires(T t) { t.solve(); };
    template<class T> concept HasEvaluate = requires(T t) { t.evaluate(); };
    template<class T> concept HasPromote = requires(T t) { t.promote(); };
    template<class T> concept HasSelectModel = requires(T t) { t.selectModel(); };
    template<class T> constexpr bool inert = !HasStep<T> && !HasSolve<T> && !HasEvaluate<T> &&
        !HasPromote<T> && !HasSelectModel<T>;
    static_assert(inert<Contract> && inert<ae::ValidityCriterionDescriptor> &&
        inert<ae::ErrorMeasureDescriptor> && inert<ae::FidelityReviewRequest> &&
        inert<ae::PhysicsChallengeCase> && inert<ae::PhysicsChallengeRegistry>);
    static_assert(std::is_same_v<ae::QuantityOfInterestDescriptor, ae::PhysicalQuantityDescriptor>);
    static_assert(std::is_same_v<decltype(std::declval<Ref&>().identifier()), const std::string&>);
    static_assert(std::is_same_v<decltype(std::declval<ae::PhysicsChallengeRegistry&>().cases()),
        const std::vector<ae::PhysicsChallengeCase>&>);
    static_assert(std::is_same_v<decltype(std::declval<ae::PhysicsChallengeRegistry&>().find("id")),
        const ae::PhysicsChallengeCase*>);
    static_assert(!std::is_copy_assignable_v<ae::PhysicsChallengeRegistry>);

    void sameContract(const Contract& a, const Contract& b)
    {
        require(a.model == b.model && a.referenceModels == b.referenceModels && a.description == b.description,
            "Model/reference metadata changed.");
        require(a.quantitiesOfInterest.size() == b.quantitiesOfInterest.size() &&
            a.validityCriteria.size() == b.validityCriteria.size() && a.errorMeasures.size() == b.errorMeasures.size(),
            "Contract collection count changed.");
        for (std::size_t i = 0; i < a.quantitiesOfInterest.size(); ++i)
        {
            const auto& x = a.quantitiesOfInterest[i]; const auto& y = b.quantitiesOfInterest[i];
            require(x.key == y.key && x.typeIdentifier == y.typeIdentifier && x.unit == y.unit &&
                x.description == y.description, "QoI content/order changed.");
        }
        for (std::size_t i = 0; i < a.validityCriteria.size(); ++i)
        {
            const auto& x = a.validityCriteria[i]; const auto& y = b.validityCriteria[i];
            require(x.key == y.key && x.criterionIdentifier == y.criterionIdentifier &&
                x.description == y.description && x.note == y.note, "Validity declaration changed.");
        }
        for (std::size_t i = 0; i < a.errorMeasures.size(); ++i)
        {
            const auto& x = a.errorMeasures[i]; const auto& y = b.errorMeasures[i];
            require(x.key == y.key && x.quantityOfInterestKey == y.quantityOfInterestKey &&
                x.measureIdentifier == y.measureIdentifier && x.unit == y.unit && x.description == y.description,
                "Numerical/reduction error declaration changed.");
        }
    }

    void sameChallenge(const ae::PhysicsChallengeCase& a, const ae::PhysicsChallengeCase& b)
    {
        require(a.identifier == b.identifier && a.model == b.model &&
            a.quantityOfInterestKeys == b.quantityOfInterestKeys && a.reasons == b.reasons &&
            a.referenceModels == b.referenceModels && a.contextReference == b.contextReference &&
            a.description == b.description, "Challenge content/order changed.");
    }
}

int main()
{
    try
    {
        const Ref reduced{"example.reduced", "draft A"};
        require(reduced.identifier() == "example.reduced" && reduced.version() == "draft A",
            "Model reference did not preserve exact identity/version.");
        require(reduced != Ref{"example.reduced", "draft B"}, "Versions were conflated.");
        for (const auto blank : {"", " \t\r\n"})
        {
            rejects([&] { Ref bad{blank, "v"}; });
            rejects([&] { Ref bad{"id", blank}; });
        }
        Contract contract{reduced,
            {{"qoi-z", "example.quantity-z", "example unit", "Scope Z"},
             {"qoi-a", "example.quantity-a", "dimensionless", std::nullopt}},
            {{"criterion-z", "example.criterion-z", "Declared regime Z", "No predicate"},
             {"criterion-a", "example.criterion-a", "Declared regime A", std::nullopt}},
            {{"measure-z", "qoi-a", "example.error-z", "dimensionless", "Future report Z"},
             {"measure-a", "qoi-z", "example.error-a", "example unit", std::nullopt}},
            {{"example.reference-z", "v2"}, {"example.reference-a", "v1"}},
            "Fixture only; no scientific certification"};
        const auto expected = contract;
        contract.validate(); contract.validate();
        sameContract(contract, expected);
        // A branching graph: two contracts can share a reference; general models
        // may have no references. Neither insertion order nor version ranks models.
        auto branch = contract;
        branch.model = Ref{"example.other-reduction", "v0"};
        branch.referenceModels = {contract.referenceModels[1]};
        branch.validate();
        auto general = contract;
        general.model = contract.referenceModels[1];
        general.referenceModels.clear(); general.validityCriteria.clear(); general.errorMeasures.clear();
        general.validate();
        require(branch.referenceModels[0] == general.model, "Shared graph edge changed.");
        sameContract(contract, expected);

        for (std::size_t i = 0; i < reasons.size(); ++i)
            for (std::size_t j = i + 1; j < reasons.size(); ++j)
                require(reasons[i] != reasons[j], "Audit reasons are not distinct.");
        ae::FidelityReviewRequest review{"review-z", reduced, contract.referenceModels,
            {"qoi-a", "qoi-z"}, {reasons.begin(), reasons.end()}, "example.snapshot", "Review only"};
        const auto expectedReview = review;
        review.validate(); review.validate(contract);
        require(review.identifier == expectedReview.identifier && review.currentModel == expectedReview.currentModel &&
            review.candidateModels == expectedReview.candidateModels &&
            review.quantityOfInterestKeys == expectedReview.quantityOfInterestKeys &&
            review.reasons == expectedReview.reasons && review.contextReference == expectedReview.contextReference &&
            review.note == expectedReview.note, "Review metadata changed.");
        auto minimalReview = review;
        minimalReview.candidateModels.clear(); minimalReview.contextReference.reset(); minimalReview.note.reset();
        minimalReview.quantityOfInterestKeys.clear(); minimalReview.validate(contract);

        ae::PhysicsChallengeCase challenge{"case-z", reduced, {"qoi-z", "qoi-a"},
            {Reason::CrossFidelityDisagreement, Reason::SuddenGainAudit}, contract.referenceModels,
            "example.reproduction:seed-and-config", "Permanent fixture artifact"};
        challenge.validate(contract);
        ae::PhysicsChallengeRegistry registry;
        require(registry.cases().empty() && !registry.find("missing"), "New registry is not empty.");
        registry.add(challenge);
        auto second = challenge; second.identifier = "case-a"; second.referenceModels.clear();
        registry.add(second);
        require(registry.cases().size() == 2, "Registry lost a case.");
        sameChallenge(registry.cases()[0], challenge);
        sameChallenge(registry.cases()[1], second);
        require(registry.find("case-a") != nullptr && !registry.find("missing"), "Read-only lookup failed.");
        sameChallenge(*registry.find("case-z"), challenge);
        auto duplicate = challenge; duplicate.description = "Must not overwrite";
        rejects([&] { registry.add(duplicate); });
        require(registry.cases().size() == 2, "Duplicate insertion changed registry size.");
        sameChallenge(*registry.find("case-z"), challenge);
        auto malformedCase = challenge; malformedCase.identifier = "";
        rejects([&] { registry.add(malformedCase); });
        require(registry.cases().size() == 2, "Rejected insertion changed registry size.");
        auto detached = challenge; detached.contextReference = "different context";
        sameChallenge(registry.cases()[0], challenge);
        sameContract(contract, expected); // Reviews/artifacts never change model contracts.

        for (const auto blank : {"", " \t\r\n"})
        {
            for (const auto field : {&ae::PhysicalQuantityDescriptor::key,
                &ae::PhysicalQuantityDescriptor::typeIdentifier, &ae::PhysicalQuantityDescriptor::unit})
                rejects([&] { auto bad = contract; bad.quantitiesOfInterest[0].*field = blank; bad.validate(); });
            for (const auto field : {&ae::ValidityCriterionDescriptor::key,
                &ae::ValidityCriterionDescriptor::criterionIdentifier, &ae::ValidityCriterionDescriptor::description})
                rejects([&] { auto bad = contract; bad.validityCriteria[0].*field = blank; bad.validate(); });
            for (const auto field : {&ae::ErrorMeasureDescriptor::key, &ae::ErrorMeasureDescriptor::quantityOfInterestKey,
                &ae::ErrorMeasureDescriptor::measureIdentifier, &ae::ErrorMeasureDescriptor::unit})
                rejects([&] { auto bad = contract; bad.errorMeasures[0].*field = blank; bad.validate(); });
            rejects([&] { auto bad = review; bad.identifier = blank; bad.validate(); });
            rejects([&] { auto bad = review; bad.contextReference = blank; bad.validate(); });
            rejects([&] { auto bad = review; bad.quantityOfInterestKeys[0] = blank; bad.validate(); });
            for (const auto field : {&ae::PhysicsChallengeCase::identifier,
                &ae::PhysicsChallengeCase::contextReference, &ae::PhysicsChallengeCase::description})
                rejects([&] { auto bad = challenge; bad.*field = blank; bad.validate(); });
            rejects([&] { auto bad = challenge; bad.quantityOfInterestKeys[0] = blank; bad.validate(); });
        }
        rejects([&] { auto bad = contract; bad.quantitiesOfInterest.clear(); bad.validate(); });
        rejects([&] { auto bad = contract; bad.quantitiesOfInterest.push_back(bad.quantitiesOfInterest[0]); bad.validate(); });
        rejects([&] { auto bad = contract; bad.validityCriteria.push_back(bad.validityCriteria[0]); bad.validate(); });
        rejects([&] { auto bad = contract; bad.errorMeasures.push_back(bad.errorMeasures[0]); bad.validate(); });
        rejects([&] { auto bad = contract; bad.errorMeasures[0].quantityOfInterestKey = "missing"; bad.validate(); });
        rejects([&] { auto bad = review; bad.reasons.clear(); bad.validate(); });
        rejects([&] { auto bad = challenge; bad.reasons.clear(); bad.validate(); });
        rejects([&] { auto bad = review; bad.reasons.push_back(static_cast<Reason>(999)); bad.validate(); });
        rejects([&] { auto bad = challenge; bad.reasons.push_back(static_cast<Reason>(999)); bad.validate(); });
        auto unscopedReview = review; unscopedReview.quantityOfInterestKeys = {"external-qoi"};
        unscopedReview.validate();
        rejects([&] { unscopedReview.validate(contract); });
        auto unscopedChallenge = challenge; unscopedChallenge.quantityOfInterestKeys = {"external-qoi"};
        unscopedChallenge.validate();
        rejects([&] { unscopedChallenge.validate(contract); });
        rejects([&] { auto bad = contract; bad.quantitiesOfInterest.clear(); review.validate(bad); });
        std::cout << "All adaptive certified physics contract tests passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
