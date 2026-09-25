#include <cmath>
#include <iostream>
#include <stdexcept>

#include "alien_evolution/development/Phenotype.hpp"
#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/evaluation/Energetics.hpp"
#include "alien_evolution/evaluation/PhenotypeMetrics.hpp"

namespace
{

    void require(
        const bool condition,
        const char* message
    )
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    bool nearlyEqual(
        const double a,
        const double b,
        const double tolerance = 1.0e-12
    )
    {
        return std::abs(a - b) <= tolerance;
    }

} // namespace

int main()
{
    try
    {
        // --------------------------------------------------------
        // Test 1:
        // One full isolated cell has material 1 and boundary 4.
        // --------------------------------------------------------

        ae::Phenotype singleCell(3, 3);
        singleCell.setMaterial(1, 1, 1.0);

        const ae::PhenotypeMetrics singleMetrics =
            ae::measurePhenotype(singleCell);

        require(
            nearlyEqual(singleMetrics.totalMaterial, 1.0),
            "Single-cell total material is incorrect."
        );

        require(
            nearlyEqual(singleMetrics.exposedBoundary, 4.0),
            "Single-cell exposed boundary is incorrect."
        );

        // --------------------------------------------------------
        // Test 2:
        // Two adjacent full cells have total boundary 6, not 8,
        // because their shared edge is internal.
        // --------------------------------------------------------

        ae::Phenotype twoCells(4, 3);
        twoCells.setMaterial(1, 1, 1.0);
        twoCells.setMaterial(2, 1, 1.0);

        const ae::PhenotypeMetrics twoMetrics =
            ae::measurePhenotype(twoCells);

        require(
            nearlyEqual(twoMetrics.totalMaterial, 2.0),
            "Two-cell total material is incorrect."
        );

        require(
            nearlyEqual(twoMetrics.exposedBoundary, 6.0),
            "Two-cell exposed boundary is incorrect."
        );

        // --------------------------------------------------------
        // Test 3:
        // A solid 2x2 block has perimeter 8.
        // --------------------------------------------------------

        ae::Phenotype block(4, 4);

        block.setMaterial(1, 1, 1.0);
        block.setMaterial(2, 1, 1.0);
        block.setMaterial(1, 2, 1.0);
        block.setMaterial(2, 2, 1.0);

        const ae::PhenotypeMetrics blockMetrics =
            ae::measurePhenotype(block);

        require(
            nearlyEqual(blockMetrics.totalMaterial, 4.0),
            "2x2 block material is incorrect."
        );

        require(
            nearlyEqual(blockMetrics.exposedBoundary, 8.0),
            "2x2 block boundary is incorrect."
        );

        // --------------------------------------------------------
        // Test 4:
        // Energetic consequences follow the configured equations.
        // --------------------------------------------------------

        ae::Environment environment{};
        environment.resourceAvailability = 2.0;

        ae::EnergeticsConfig energetics{};
        energetics.resourceGainPerBoundary = 0.5;
        energetics.maintenanceCostPerMaterial = 0.25;

        const ae::EnergeticConsequences consequences =
            ae::evaluateEnergetics(
                blockMetrics,
                environment,
                energetics
            );

        // Acquisition:
        // 8 boundary * 2 resources * 0.5 gain = 8
        require(
            nearlyEqual(
                consequences.resourceAcquisition,
                8.0
            ),
            "Resource acquisition is incorrect."
        );

        // Maintenance:
        // 4 material * 0.25 = 1
        require(
            nearlyEqual(
                consequences.maintenanceCost,
                1.0
            ),
            "Maintenance cost is incorrect."
        );

        require(
            nearlyEqual(
                consequences.netEnergy,
                7.0
            ),
            "Net energy is incorrect."
        );

        std::cout
            << "All evaluation tests passed.\n";

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr
            << "TEST FAILURE: "
            << error.what()
            << '\n';

        return 1;
    }
}