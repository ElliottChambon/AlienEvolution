#include "alien_evolution/science/ScientificModelMetadata.hpp"

#include <array>
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace
{
    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    template<class Function>
    void rejects(Function function)
    {
        try { function(); }
        catch (const std::invalid_argument&) { return; }
        throw std::runtime_error("Malformed metadata was accepted.");
    }

    template<class T> concept Ordered = requires(T a, T b)
    {
        a < b; a <= b; a > b; a >= b;
    };
    static_assert(!Ordered<ae::EvidenceStatus>);
    static_assert(!std::is_convertible_v<ae::EvidenceStatus, int>);
    static_assert(!std::is_same_v<ae::EvidenceStatus, ae::UncertaintyKind>);
    static_assert(ae::UncertaintyKind::NumericalReduction != ae::UncertaintyKind::ScientificModelForm);
}

int main()
{
    try
    {
        using E = ae::EvidenceStatus;
        const std::array statuses{E::EstablishedPhysicalInteraction,
            E::DemonstratedBiologicalSensoryUse, E::DemonstratedBiologicalResponse,
            E::UnresolvedMechanism, E::MechanisticallyGroundedExtrapolation, E::Speculative};
        const std::array<std::string_view, 6> codes{"P1", "B1", "B2", "M", "X", "S"};
        for (std::size_t i = 0; i < codes.size(); ++i)
        {
            require(ae::evidenceCode(statuses[i]) == codes[i], "Incorrect evidence code.");
            require(ae::evidenceStatusFromCode(codes[i]) == statuses[i], "Incorrect inverse mapping.");
        }
        for (const auto code : {"", "p1", " B1", "S ", "unknown"})
            require(!ae::evidenceStatusFromCode(code), "Accepted an unknown/nonexact code.");
        rejects([] { (void)ae::evidenceCode(static_cast<E>(999)); });

        ae::ScientificModelMetadata model;
        model.identifier = "example-model";
        model.name = "Metadata test fixture";
        model.version = "draft revision A"; // No semantic-version requirement.
        model.description = "Software fixture; no mechanism certification.";
        model.supersedes = {"older-b", "older-a"};
        model.alternatives = {"alternative-z", "alternative-a"};
        model.evidence = {
            {E::EstablishedPhysicalInteraction, "Example physical layer",
                {{"Second label", "doi:fixture-2", "Scope two"},
                 {"First label", "url:fixture-1", std::nullopt}}, "Limited scope"},
            {E::UnresolvedMechanism, "Example unresolved layer", {}, "No supporting references yet"}
        };
        model.parameters = {{"time", "s", "Elapsed time"},
                            {"fraction", "dimensionless", std::nullopt}};
        model.assumptions = {"Assumption B", "Assumption A"};
        model.validityScope = "Metadata fixture only; not a runtime validity predicate.";
        model.uncertainties = {ae::UncertaintyKind::ScientificModelForm,
                               ae::UncertaintyKind::NumericalReduction};
        const auto copy = model;
        model.validate();
        model.validate();
        require(model.identifier == "example-model" && model.name == copy.name &&
            model.version == "draft revision A" && model.description == copy.description,
            "Identity/version changed.");
        require(model.supersedes == copy.supersedes && model.alternatives == copy.alternatives &&
            model.assumptions == copy.assumptions && model.validityScope == copy.validityScope &&
            model.uncertainties == copy.uncertainties, "Metadata order/content changed.");
        require(model.evidence.size() == 2 && model.evidence[0].status == E::EstablishedPhysicalInteraction &&
            model.evidence[1].status == E::UnresolvedMechanism &&
            model.evidence[0].claim == copy.evidence[0].claim &&
            model.evidence[0].limitation == copy.evidence[0].limitation &&
            model.evidence[1].references.empty(), "Evidence layers changed.");
        const auto& refs = model.evidence[0].references;
        require(refs.size() == 2 && refs[0].label == "Second label" && refs[1].label == "First label" &&
            refs[0].source == "doi:fixture-2" && refs[1].source == "url:fixture-1" &&
            refs[0].note == "Scope two" && !refs[1].note, "Literature order/content changed.");
        require(model.parameters.size() == 2 && model.parameters[0].key == "time" &&
            model.parameters[0].unit == "s" && model.parameters[0].description == "Elapsed time" &&
            model.parameters[1].key == "fraction" && model.parameters[1].unit == "dimensionless",
            "Parameter metadata changed.");

        auto other = copy;
        other.identifier = "alternative-z";
        model.alternatives.push_back(other.identifier);
        model.supersedes.push_back(other.identifier);
        model.validate();
        require(other.identifier == "alternative-z" && other.version == copy.version &&
            other.alternatives == copy.alternatives && other.supersedes == copy.supersedes,
            "Relationships changed another object.");
        model.version = "next";
        require(other.version == "draft revision A", "Objects unexpectedly share mutable identity.");

        // Optional scientific content may be absent; syntax is not certification.
        ae::ScientificModelMetadata minimal;
        minimal.identifier = "prototype"; minimal.name = "Prototype"; minimal.version = "unreviewed";
        minimal.validate();
        ae::LiteratureReference{"Label only", "", std::nullopt}.validate();
        ae::LiteratureReference{"", "source only", std::nullopt}.validate();
        for (const auto blank : {"", " \t\r\n"})
        {
            rejects([&] { auto bad = copy; bad.identifier = blank; bad.validate(); });
            rejects([&] { auto bad = copy; bad.name = blank; bad.validate(); });
            rejects([&] { auto bad = copy; bad.version = blank; bad.validate(); });
            rejects([&] { auto bad = copy; bad.parameters[0].key = blank; bad.validate(); });
            rejects([&] { auto bad = copy; bad.parameters[0].unit = blank; bad.validate(); });
            rejects([&] { auto bad = copy; bad.evidence[0].claim = blank; bad.validate(); });
            rejects([&] { auto bad = copy; bad.evidence[0].references[0] = {blank, blank, "Note"}; bad.validate(); });
            rejects([&] { auto bad = copy; bad.alternatives.push_back(blank); bad.validate(); });
            rejects([&] { auto bad = copy; bad.supersedes.push_back(blank); bad.validate(); });
        }
        rejects([&] { auto bad = copy; bad.evidence[0].status = static_cast<E>(999); bad.validate(); });
        rejects([&] { auto bad = copy; bad.uncertainties.push_back(static_cast<ae::UncertaintyKind>(999)); bad.validate(); });
        std::cout << "All scientific metadata tests passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
