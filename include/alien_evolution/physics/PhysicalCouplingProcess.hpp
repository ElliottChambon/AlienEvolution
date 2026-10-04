#pragma once

#include <optional>
#include <string>
#include <vector>

#include "alien_evolution/science/ScientificModelMetadata.hpp"

namespace ae
{
    // Meaning of typeIdentifier belongs to the enclosing scientific model
    // identifier/version, not to a universal physics or biological catalog.
    // Unit text documents the quantity; it does not establish compatibility.
    struct PhysicalQuantityDescriptor
    {
        std::string key;
        std::string typeIdentifier;
        std::string unit;
        std::optional<std::string> description;
        void validate() const;
    };

    // Direction at the process software boundary, not global physical causality.
    enum class PortDirectionality
    {
        Input,
        Output,
        Bidirectional
    };

    struct PhysicalPortDescriptor
    {
        std::string key;
        PhysicalQuantityDescriptor quantity;
        PortDirectionality directionality{};
        void validate() const;
    };

    // Simulator-owned schema vocabulary only; never inherited organism state.
    // No values, connections, execution, solver selection or certification.
    // All collections preserve insertion order and may be empty. Keys are
    // unique within each category, not across categories. A port's quantity
    // key is local to that port; the port key identifies the boundary entry.
    struct PhysicalCouplingProcessSchema
    {
        std::string identifier;
        ScientificModelMetadata model;
        std::optional<std::string> description;
        std::vector<PhysicalPortDescriptor> ports;
        std::vector<PhysicalQuantityDescriptor> state;
        std::vector<PhysicalQuantityDescriptor> history;
        std::vector<PhysicalQuantityDescriptor> control;
        std::vector<PhysicalQuantityDescriptor> observables;
        std::vector<PhysicalQuantityDescriptor> ledger;
        // Inert optional association; no geometry representation or resolution.
        std::optional<std::string> regionBindingIdentifier;

        // Explicit read-only syntax check after assembling/editing the schema.
        // State/history declarations impose no Markovian or integrator assumption.
        void validate() const;
    };
} // namespace ae
