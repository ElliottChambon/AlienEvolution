#include "alien_evolution/science/ScientificModelMetadata.hpp"

#include <stdexcept>

namespace
{
    bool blank(const std::string_view text)
    {
        return text.find_first_not_of(" \t\n\r\f\v") == std::string_view::npos;
    }

    void requireText(const std::string_view text, const char* message)
    {
        if (blank(text))
            throw std::invalid_argument(message);
    }
}

namespace ae
{
    std::string_view evidenceCode(const EvidenceStatus status)
    {
        switch (status)
        {
        case EvidenceStatus::EstablishedPhysicalInteraction: return "P1";
        case EvidenceStatus::DemonstratedBiologicalSensoryUse: return "B1";
        case EvidenceStatus::DemonstratedBiologicalResponse: return "B2";
        case EvidenceStatus::UnresolvedMechanism: return "M";
        case EvidenceStatus::MechanisticallyGroundedExtrapolation: return "X";
        case EvidenceStatus::Speculative: return "S";
        }
        throw std::invalid_argument("Unknown evidence status.");
    }

    std::optional<EvidenceStatus> evidenceStatusFromCode(const std::string_view code)
    {
        if (code == "P1") return EvidenceStatus::EstablishedPhysicalInteraction;
        if (code == "B1") return EvidenceStatus::DemonstratedBiologicalSensoryUse;
        if (code == "B2") return EvidenceStatus::DemonstratedBiologicalResponse;
        if (code == "M") return EvidenceStatus::UnresolvedMechanism;
        if (code == "X") return EvidenceStatus::MechanisticallyGroundedExtrapolation;
        if (code == "S") return EvidenceStatus::Speculative;
        return std::nullopt;
    }

    void LiteratureReference::validate() const
    {
        if (blank(label) && blank(source))
            throw std::invalid_argument("Literature reference requires a label or source.");
    }

    void EvidenceRecord::validate() const
    {
        (void)evidenceCode(status);
        requireText(claim, "Evidence requires a claim/scope.");
        for (const auto& reference : references) reference.validate();
    }

    void ParameterMetadata::validate() const
    {
        requireText(key, "Parameter requires a key.");
        requireText(unit, "Parameter requires a unit string (use 'dimensionless' when appropriate).");
    }

    void ScientificModelMetadata::validate() const
    {
        requireText(identifier, "Model requires an identifier.");
        requireText(name, "Model requires a name.");
        requireText(version, "Model requires a version.");
        for (const auto& id : supersedes) requireText(id, "Empty superseded model identifier.");
        for (const auto& id : alternatives) requireText(id, "Empty alternative model identifier.");
        for (const auto& record : evidence) record.validate();
        for (const auto& parameter : parameters) parameter.validate();
        for (const auto kind : uncertainties)
        {
            switch (kind)
            {
            case UncertaintyKind::NumericalReduction:
            case UncertaintyKind::ScientificModelForm: break;
            default: throw std::invalid_argument("Unknown uncertainty kind.");
            }
        }
    }
} // namespace ae
