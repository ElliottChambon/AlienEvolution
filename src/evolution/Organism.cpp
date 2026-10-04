#include "alien_evolution/evolution/Organism.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace ae
{

    Organism::Organism(HeritableConstructionState constructionState)
        : constructionState_(std::move(constructionState))
    {}

    Organism::Organism(HeritableProgram heritableProgram)
        : Organism(HeritableConstructionState::fromPrototypeHeritableProgram(std::move(heritableProgram)))
    {}

    const HeritableConstructionState& Organism::constructionState() const
    {
        return constructionState_;
    }

    Organism::Organism(
        RegulatoryProgram regulatoryProgram
    )
        : Organism(HeritableProgram(std::move(regulatoryProgram)))
    {}

    const HeritableProgram& Organism::heritableProgram() const
    {
        return constructionState_.prototypeHeritableProgram();
    }

    const RegulatoryProgram&
        Organism::regulatoryProgram() const
    {
        return heritableProgram().regulatoryProgram();
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

    void Organism::setPhenotype(
        Phenotype phenotype
    )
    {
        phenotype_ =
            std::move(
                phenotype
            );
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

    void Organism::setFitness(
        const double fitness
    )
    {
        if (
            !std::isfinite(fitness)
            || fitness < 0.0
            )
        {
            throw std::invalid_argument(
                "Organism fitness must be finite and non-negative."
            );
        }

        fitness_ =
            fitness;
    }

    void Organism::clearEvaluation()
    {
        phenotype_.reset();
        fitness_.reset();
    }

} // namespace ae
