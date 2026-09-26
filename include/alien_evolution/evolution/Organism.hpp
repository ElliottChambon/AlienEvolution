#pragma once

#include <optional>

#include "alien_evolution/development/Phenotype.hpp"
#include "alien_evolution/genetics/RegulatoryProgram.hpp"

namespace ae
{

    class Organism
    {
    public:
        explicit Organism(
            RegulatoryProgram regulatoryProgram
        );

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
        RegulatoryProgram regulatoryProgram_;

        std::optional<Phenotype> phenotype_;

        std::optional<double> fitness_;
    };

} // namespace ae