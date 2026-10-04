#include "alien_evolution/genetics/HeritableMutation.hpp"

#include <utility>

namespace ae
{
    HeritableMutationGenerationResult generateHeritableOffspring(
        const HeritableProgram& parent, const HeritableMutationConfig& config, Random& random)
    {
        auto regulatory = generateRegulatoryOffspring(parent.regulatoryProgram(), config.regulatory, random);
        auto sensory = generateSensoryOffspring(parent.sensoryProgram(), config.sensory, random);
        return {HeritableProgram(std::move(regulatory.offspringProgram), std::move(sensory.offspringProgram)),
            std::move(regulatory.events), std::move(sensory.events)};
    }
} // namespace ae
