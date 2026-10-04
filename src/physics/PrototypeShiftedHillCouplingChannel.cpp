#include "alien_evolution/physics/PrototypeShiftedHillCouplingChannel.hpp"

#include "alien_evolution/genetics/RegulatoryDynamics.hpp"

namespace
{
    ae::PhysicalCouplingProcessSchema makeSchema()
    {
        ae::PhysicalCouplingProcessSchema schema;
        schema.identifier = "alien_evolution.prototype.shifted_hill.channel";
        auto& model = schema.model;
        model.identifier = "alien_evolution.prototype.shifted_hill_pcc";
        model.name = "Prototype reduced shifted-Hill coupling channel";
        model.version = "m6-v1";
        model.description = "Phenomenological deterministic memoryless PCC reproducing the V0.3 "
            "RegulatoryInputInterface single-channel response using RegulatoryDynamics::shiftedHill. "
            "Provenance: docs/SCIENTIFIC_DEBT.md external signal transduction (V0.3B1), "
            "src/genetics/RegulatoryInput.cpp and src/genetics/RegulatoryDynamics.cpp. "
            "Architecture/migration verification only, not biological calibration or a full PCP. "
            "No stronger scientific evidence category is assigned by this migration.";
        model.parameters = {
            {"foldChange", "dimensionless", "Finite positive asymptotic modulation parameter"},
            {"halfSaturation", "caller/model-defined input unit", "Finite positive input scale, same unit as input"},
            {"cooperativity", "dimensionless", "Finite positive phenomenological exponent"}
        };
        model.assumptions = {
            "One deterministic finite nonnegative scalar input in caller/model-defined units",
            "Finite positive foldChange, halfSaturation and cooperativity; constant during evaluation",
            "No internal state/history or control; memoryless response",
            "No physical transport/access, receptor chemistry or material coupling",
            "No intrinsic physical noise, adaptation or energetic/material exchange accounting",
            "Mathematical output is positive; legacy floating-point cancellation can round extreme repression to zero"
        };
        model.validityScope = "Single-channel V0.3 mathematical compatibility only under the declared scalar "
            "assumptions; no claim of physical or biological adequacy. Legacy finite-precision behavior is retained.";
        model.uncertainties = {ae::UncertaintyKind::NumericalReduction, ae::UncertaintyKind::ScientificModelForm};
        schema.description = "Reduced PCC described with M2 schema vocabulary; not executable general PCP semantics";
        schema.ports = {{"scalar_input", {"input", "prototype.shifted_hill.external_scalar",
            "caller/model-defined input unit", "Finite nonnegative scalar; no physical-domain meaning assigned"},
            ae::PortDirectionality::Input}};
        schema.observables = {{"modulation_factor", "prototype.shifted_hill.modulation_factor",
            "dimensionless", "Single-channel regulatory modulation, with legacy finite-precision semantics"}};
        schema.validate();
        return schema;
    }

    ae::AdaptivePhysicsContract makeContract(const ae::PhysicalCouplingProcessSchema& schema)
    {
        ae::AdaptivePhysicsContract contract{
            ae::ScientificModelRef{schema.model.identifier, schema.model.version},
            schema.observables, {}, {}, {},
            "Declared prototype regime only; no executable validity predicate or scientific certification"
        };
        contract.validityCriteria = {{"scalar_regime", "prototype.shifted_hill.scalar_regime",
            "Deterministic finite nonnegative scalar input; finite positive constant parameters; "
            "no internal state/history or control; no transport/access physics, intrinsic physical noise, "
            "energetic/material exchanges, receptor/material microphysics or adaptation.",
            "Declaration only. Input/parameter syntax checks do not certify scientific validity."}};
        contract.errorMeasures = {{"legacy_compatibility", "modulation_factor",
            "prototype.shifted_hill.legacy_output_difference", "dimensionless",
            "Numerical compatibility versus the V0.3 mathematical implementation only. "
            "Bitwise equivalence is tested; no runtime error estimator is implemented. "
            "Zero migration difference does not imply zero scientific/model-form uncertainty."}};
        contract.referenceModels = {{"alien_evolution.prototype.v03.shifted_hill_external_input", "v0.3"}};
        contract.validate();
        return contract;
    }
}

namespace ae
{
    PrototypeShiftedHillCouplingChannel::PrototypeShiftedHillCouplingChannel(
        const double foldChange, const double halfSaturation, const double cooperativity)
        : foldChange_(foldChange), halfSaturation_(halfSaturation), cooperativity_(cooperativity)
    {
        // Reuse the existing finite/positive parameter checks as well as its
        // numerical kernel. No independent shifted-Hill law is introduced.
        (void)RegulatoryDynamics::shiftedHill(0.0, halfSaturation_, cooperativity_, foldChange_);
    }

    double PrototypeShiftedHillCouplingChannel::evaluate(const double input) const
    {
        return RegulatoryDynamics::shiftedHill(input, halfSaturation_, cooperativity_, foldChange_);
    }

    double PrototypeShiftedHillCouplingChannel::foldChange() const { return foldChange_; }
    double PrototypeShiftedHillCouplingChannel::halfSaturation() const { return halfSaturation_; }
    double PrototypeShiftedHillCouplingChannel::cooperativity() const { return cooperativity_; }

    const ScientificModelMetadata& PrototypeShiftedHillCouplingChannel::scientificMetadata() const
    {
        return schema().model;
    }

    const PhysicalCouplingProcessSchema& PrototypeShiftedHillCouplingChannel::schema() const
    {
        static const auto descriptor = makeSchema();
        return descriptor;
    }

    const AdaptivePhysicsContract& PrototypeShiftedHillCouplingChannel::adaptivePhysicsContract() const
    {
        static const auto contract = makeContract(schema());
        return contract;
    }
} // namespace ae
