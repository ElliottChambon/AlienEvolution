#include <iostream>

#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/genetics/Genome.hpp"

int main()
{
    const ae::Environment environment{};
    const ae::Genome genome{};

    std::cout << "AlienEvolution v0.1.0\n\n";

    std::cout << "Environment\n";
    std::cout << "  Gravity: "
        << environment.gravity
        << " m/s^2\n";

    std::cout << "  Resources: "
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

    return 0;
}