#include "alien_evolution/physics/PhysicalCouplingProcess.hpp"

#include <set>
#include <stdexcept>
#include <string_view>

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
                throw std::invalid_argument("Duplicate local key within schema category.");
        }
    }
}

namespace ae
{
    void PhysicalQuantityDescriptor::validate() const
    {
        requireText(key, "Quantity requires a local key.");
        requireText(typeIdentifier, "Quantity requires a model-owned type identifier.");
        requireText(unit, "Quantity requires a unit string.");
    }

    void PhysicalPortDescriptor::validate() const
    {
        requireText(key, "Port requires a key.");
        quantity.validate();
        switch (directionality)
        {
        case PortDirectionality::Input:
        case PortDirectionality::Output:
        case PortDirectionality::Bidirectional: break;
        default: throw std::invalid_argument("Unknown port directionality.");
        }
    }

    void PhysicalCouplingProcessSchema::validate() const
    {
        requireText(identifier, "Process schema requires an identifier.");
        model.validate();
        validateCategory(ports);
        validateCategory(state);
        validateCategory(history);
        validateCategory(control);
        validateCategory(observables);
        validateCategory(ledger);
        if (regionBindingIdentifier)
            requireText(*regionBindingIdentifier, "Present region binding requires an identifier.");
    }
} // namespace ae
