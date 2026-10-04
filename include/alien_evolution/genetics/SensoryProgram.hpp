#pragma once

#include <cstddef>
#include <vector>

#include "alien_evolution/genetics/RegulatoryInput.hpp"

namespace ae
{
    // Inherited channel data, distinct from the validated runtime adapter.
    // ID-based shifted-Hill channels are a temporary phenomenological model.
    class SensoryProgram
    {
    public:
        explicit SensoryProgram(std::vector<RegulatoryInputChannel> channels = {});

        [[nodiscard]] const std::vector<RegulatoryInputChannel>& channels() const;
        [[nodiscard]] std::size_t channelCount() const;

        // Target validation is deferred until an adapter is requested. B4
        // preserves channels unchanged even if regulatory mutation removes a target.
        [[nodiscard]] RegulatoryInputInterface makeRegulatoryInputInterface(
            const RegulatoryProgram& program
        ) const;

    private:
        std::vector<RegulatoryInputChannel> channels_;
    };
} // namespace ae
