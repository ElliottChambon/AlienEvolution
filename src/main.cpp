#include <iostream>

#include "alien_evolution/development/Development.hpp"
#include "alien_evolution/development/Phenotype.hpp"
#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/genetics/Genome.hpp"

namespace
{

    void printPhenotype(const ae::Phenotype& phenotype)
    {
        for (std::size_t y = 0; y < phenotype.height(); ++y)
        {
            for (std::size_t x = 0; x < phenotype.width(); ++x)
            {
                const double material =
                    phenotype.materialAt(x, y);

                char symbol = ' ';

                if (material >= 0.75)
                {
                    symbol = '#';
                }
                else if (material >= 0.50)
                {
                    symbol = 'O';
                }
                else if (material >= 0.25)
                {
                    symbol = 'o';
                }
                else if (material > 0.0)
                {
                    symbol = '.';
                }

                std::cout << symbol;
            }

            std::cout << '\n';
        }
    }

} // namespace

int main()
{
    const ae::Environment environment{};
    const ae::Genome genome{};

    const ae::Development development(
        40,
        30,
        10
    );

    const ae::Phenotype phenotype =
        development.develop(
            genome,
            environment
        );

    std::cout << "AlienEvolution v0.1.0\n\n";

    std::cout << "Environment\n";
    std::cout
        << "  Gravity: "
        << environment.gravity
        << " m/s^2\n";

    std::cout
        << "  Resources: "
        << environment.resourceAvailability
        << "\n\n";

    std::cout << "Genome\n";
    std::cout << "  alphaR: " << genome.alphaR << '\n';
    std::cout << "  alphaE: " << genome.alphaE << '\n';
    std::cout << "  theta: " << genome.theta << '\n';
    std::cout << "  lambda: " << genome.lambda << '\n';
    std::cout << "  beta: " << genome.beta << '\n';
    std::cout << "  growthRate: " << genome.growthRate << '\n';
    std::cout << "  metabolicCost: " << genome.metabolicCost << '\n';

    std::cout << "\nDeveloped phenotype\n";
    std::cout
        << "  Grid: "
        << phenotype.width()
        << " x "
        << phenotype.height()
        << '\n';

    std::cout
        << "  Total material: "
        << phenotype.totalMaterial()
        << "\n\n";

    printPhenotype(phenotype);

    return 0;
}