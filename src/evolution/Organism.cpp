#include "alien_evolution/evolution/Organism.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace ae
{

    Organism::Organism(Genome genome)
        : genome_(std::move(genome))
    {}

    const Genome& Organism::genome() const
    {
        return genome_;
    }

    bool Organism::hasPhenotype() const
    {
        return phenotype_.has_value();
    }

    const Phenotype& Organism::phenotype() const
    {
        if (!phenotype_)
        {
            throw std::logic_error(
                "Organism phenotype has not been developed."
            );
        }

        return *phenotype_;
    }

    void Organism::setPhenotype(Phenotype phenotype)
    {
        phenotype_ = std::move(phenotype);
    }

    bool Organism::hasFitness() const
    {
        return fitness_.has_value();
    }

    double Organism::fitness() const
    {
        if (!fitness_)
        {
            throw std::logic_error(
                "Organism fitness has not been evaluated."
            );
        }

        return *fitness_;
    }

    void Organism::setFitness(const double fitness)
    {
        if (!std::isfinite(fitness) || fitness < 0.0)
        {
            throw std::invalid_argument(
                "Organism fitness must be finite and non-negative."
            );
        }

        fitness_ = fitness;
    }

    void Organism::clearEvaluation()
    {
        phenotype_.reset();
        fitness_.reset();
    }

} // namespace ae