#pragma once

#include <optional>

#include "alien_evolution/development/Phenotype.hpp"
#include "alien_evolution/genetics/HeritableConstructionState.hpp"

namespace ae
{

    class Organism
    {
    public:
        explicit Organism(HeritableConstructionState constructionState);

        [[nodiscard]] const HeritableConstructionState& constructionState() const;

        // Prototype compatibility construction; current mutation/reproduction
        // continue to operate on the HCS-owned HeritableProgram payload.
        explicit Organism(HeritableProgram heritableProgram);

        // Temporary migration aid: regulatory-only organisms have empty sensing.
        explicit Organism(
            RegulatoryProgram regulatoryProgram
        );

        [[nodiscard]]
        const HeritableProgram& heritableProgram() const;

        // Temporary compatibility accessor for regulatory-only callers.
        [[nodiscard]]
        const RegulatoryProgram& regulatoryProgram() const;

        [[nodiscard]]
        bool hasPhenotype() const;

        [[nodiscard]]
        const Phenotype& phenotype() const;

        void setPhenotype(
            Phenotype phenotype
        );

        [[nodiscard]]
        bool hasFitness() const;

        [[nodiscard]]
        double fitness() const;

        void setFitness(
            double fitness
        );

        void clearEvaluation();

    private:
        HeritableConstructionState constructionState_;

        std::optional<Phenotype> phenotype_;

        std::optional<double> fitness_;
    };

} // namespace ae
