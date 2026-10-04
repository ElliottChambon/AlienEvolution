#include "alien_evolution/physics/PhysicalCouplingProcess.hpp"

#include <array>
#include <iostream>
#include <stdexcept>

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
        throw std::runtime_error("Malformed schema was accepted.");
    }

    using Schema = ae::PhysicalCouplingProcessSchema;
    using Quantity = ae::PhysicalQuantityDescriptor;
    using Direction = ae::PortDirectionality;
    static_assert(Direction::Input != Direction::Output);
    static_assert(Direction::Input != Direction::Bidirectional);
    static_assert(Direction::Output != Direction::Bidirectional);

    template<class T> concept HasStep = requires(T t) { t.step(); };
    template<class T> concept HasSolve = requires(T t) { t.solve(); };
    template<class T> concept HasEvaluate = requires(T t) { t.evaluate(); };
    template<class T> concept HasConnect = requires(T a, T b) { a.connect(b); };
    template<class T> concept HasCompatibility = requires(T a, T b) { a.isCompatibleWith(b); };
    static_assert(!HasStep<Schema> && !HasSolve<Schema> && !HasEvaluate<Schema>);
    static_assert(!HasConnect<ae::PhysicalPortDescriptor> && !HasCompatibility<ae::PhysicalPortDescriptor>);

    void sameQuantity(const Quantity& actual, const Quantity& expected)
    {
        require(actual.key == expected.key && actual.typeIdentifier == expected.typeIdentifier &&
            actual.unit == expected.unit && actual.description == expected.description,
            "Quantity content/order changed.");
    }

    void sameModel(const ae::ScientificModelMetadata& a, const ae::ScientificModelMetadata& b)
    {
        require(a.identifier == b.identifier && a.name == b.name && a.version == b.version &&
            a.description == b.description && a.supersedes == b.supersedes &&
            a.alternatives == b.alternatives && a.assumptions == b.assumptions &&
            a.validityScope == b.validityScope && a.uncertainties == b.uncertainties,
            "Model metadata changed.");
        require(a.evidence.size() == b.evidence.size(), "Evidence count changed.");
        for (std::size_t i = 0; i < a.evidence.size(); ++i)
        {
            const auto& x = a.evidence[i]; const auto& y = b.evidence[i];
            require(x.status == y.status && x.claim == y.claim && x.limitation == y.limitation &&
                x.references.size() == y.references.size(), "Evidence changed.");
            for (std::size_t j = 0; j < x.references.size(); ++j)
                require(x.references[j].label == y.references[j].label &&
                    x.references[j].source == y.references[j].source &&
                    x.references[j].note == y.references[j].note, "Literature changed.");
        }
        require(a.parameters.size() == b.parameters.size(), "Parameter count changed.");
        for (std::size_t i = 0; i < a.parameters.size(); ++i)
            require(a.parameters[i].key == b.parameters[i].key && a.parameters[i].unit == b.parameters[i].unit &&
                a.parameters[i].description == b.parameters[i].description, "Parameters changed.");
    }

    constexpr std::array categories{&Schema::state, &Schema::history, &Schema::control,
                                    &Schema::observables, &Schema::ledger};
}

int main()
{
    try
    {
        Schema minimal;
        minimal.identifier = "example.process";
        minimal.model.identifier = "example.model";
        minimal.model.name = "Schema fixture";
        minimal.model.version = "draft A";
        minimal.validate(); // All descriptor categories and region may be absent.

        auto schema = minimal;
        schema.model.description = "Metadata only";
        schema.model.supersedes = {"example.old.z", "example.old.a"};
        schema.model.alternatives = {"example.other.z", "example.other.a"};
        schema.model.evidence = {
            {ae::EvidenceStatus::Speculative, "Fixture only",
                {{"Label Z", "example:source-z", "Scope Z"}, {"Label A", "example:source-a", std::nullopt}}, "No certification"},
            {ae::EvidenceStatus::UnresolvedMechanism, "Unresolved fixture", {}, std::nullopt}
        };
        schema.model.parameters = {{"parameter-z", "example unit", "Description Z"},
                                   {"parameter-a", "dimensionless", std::nullopt}};
        schema.model.assumptions = {"Assumption Z", "Assumption A"};
        schema.model.validityScope = "Test fixture only";
        schema.model.uncertainties = {ae::UncertaintyKind::ScientificModelForm, ae::UncertaintyKind::NumericalReduction};
        schema.description = "Schema with history, no dynamics";
        schema.regionBindingIdentifier = "example.region";
        schema.ports = {
            {"port-z", {"quantity", "example.flux", "example unit", "First port"}, Direction::Input},
            {"port-a", {"quantity", "example.flux", "example unit", "Second port"}, Direction::Output},
            {"port-m", {"quantity", "example.potential", "dimensionless", std::nullopt}, Direction::Bidirectional}
        };
        std::size_t index = 0;
        for (const auto category : categories)
        {
            // Reusing keys across categories is unambiguous; distinct content
            // demonstrates that state/history/control/observables/ledger stay separate.
            schema.*category = {{"local-z", "example.category-" + std::to_string(index), "example unit", "First"},
                                {"local-a", "example.other-" + std::to_string(index++), "dimensionless", std::nullopt}};
        }
        const auto expected = schema;
        schema.validate();
        schema.validate();
        require(schema.identifier == expected.identifier && schema.description == expected.description &&
            schema.regionBindingIdentifier == expected.regionBindingIdentifier, "Process metadata changed.");
        sameModel(schema.model, expected.model);
        require(schema.ports.size() == expected.ports.size(), "Port count changed.");
        for (std::size_t i = 0; i < schema.ports.size(); ++i)
        {
            require(schema.ports[i].key == expected.ports[i].key &&
                schema.ports[i].directionality == expected.ports[i].directionality, "Port order/direction changed.");
            sameQuantity(schema.ports[i].quantity, expected.ports[i].quantity);
        }
        // Equal type/unit strings on input/output remain inert descriptors.
        for (const auto category : categories)
        {
            require((schema.*category).size() == 2, "Category count changed.");
            for (std::size_t i = 0; i < 2; ++i)
                sameQuantity((schema.*category)[i], (expected.*category)[i]);
        }
        auto separate = schema;
        separate.model.version = "draft B";
        separate.history.clear();
        sameModel(schema.model, expected.model);
        require(schema.history.size() == 2 && schema.state.size() == 2, "Schema copies share state/history.");

        for (const auto blank : {"", " \t\r\n"})
        {
            rejects([&] { auto bad = schema; bad.identifier = blank; bad.validate(); });
            rejects([&] { auto bad = schema; bad.regionBindingIdentifier = blank; bad.validate(); });
            for (const auto field : {&ae::ScientificModelMetadata::identifier,
                                    &ae::ScientificModelMetadata::name, &ae::ScientificModelMetadata::version})
                rejects([&] { auto bad = schema; bad.model.*field = blank; bad.validate(); });
            rejects([&] { auto bad = schema; bad.model.evidence[0].claim = blank; bad.validate(); });
            rejects([&] { auto bad = schema; bad.model.parameters[0].key = blank; bad.validate(); });
            rejects([&] { auto bad = schema; bad.ports[0].key = blank; bad.validate(); });
            for (const auto field : {&Quantity::key, &Quantity::typeIdentifier, &Quantity::unit})
            {
                rejects([&] { auto bad = schema; bad.ports[0].quantity.*field = blank; bad.validate(); });
                for (const auto category : categories)
                    rejects([&] { auto bad = schema; (bad.*category)[0].*field = blank; bad.validate(); });
            }
        }
        rejects([&] { auto bad = schema; bad.ports[0].directionality = static_cast<Direction>(999); bad.validate(); });
        rejects([&] { auto bad = schema; bad.ports.push_back(bad.ports.front()); bad.validate(); });
        for (const auto category : categories)
            rejects([&] { auto bad = schema; (bad.*category).push_back((bad.*category).front()); bad.validate(); });
        // No scientific inference from units/types: arbitrary nonblank text is valid.
        auto uninterpreted = minimal;
        uninterpreted.ledger = {{"example.term", "example.unknown", "uninterpreted unit", std::nullopt}};
        uninterpreted.validate();
        std::cout << "All physical coupling process schema tests passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
