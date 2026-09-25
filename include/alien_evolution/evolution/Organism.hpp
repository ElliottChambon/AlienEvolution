#pragma once

#include <optional>

#include "alien_evolution/development/Phenotype.hpp"
#include "alien_evolution/genetics/Genome.hpp"

namespace ae
{

    class Organism
    {
    public:
        explicit Organism(Genome genome);

        [[nodiscard]] const Genome& genome() const;

        [[nodiscard]] bool hasPhenotype() const;
        [[nodiscard]] const Phenotype& phenotype() const;

        void setPhenotype(Phenotype phenotype);

        [[nodiscard]] bool hasFitness() const;
        [[nodiscard]] double fitness() const;

        void setFitness(double fitness);

        void clearEvaluation();

    private:
        Genome genome_;

        std::optional<Phenotype> phenotype_;
        std::optional<double> fitness_;
    };

} // namespace ae