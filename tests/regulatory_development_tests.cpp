#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>

#include "alien_evolution/development/RegulatoryDevelopment.hpp"
#include "alien_evolution/environment/Environment.hpp"
#include "alien_evolution/genetics/RegulatoryProgram.hpp"

namespace
{

    void require(
        const bool condition,
        const char* message
    )
    {
        if (!condition)
        {
            throw std::runtime_error(
                message
            );
        }
    }

    bool phenotypesEqual(
        const ae::Phenotype& a,
        const ae::Phenotype& b,
        const double tolerance = 1.0e-12
    )
    {
        if (
            a.width() != b.width()
            || a.height() != b.height()
            )
        {
            return false;
        }

        for (std::size_t y = 0;
            y < a.height();
            ++y)
        {
            for (std::size_t x = 0;
                x < a.width();
                ++x)
            {
                if (
                    std::abs(
                        a.materialAt(x, y)
                        - b.materialAt(x, y)
                    )
                > tolerance
                    )
                {
                    return false;
                }
            }
        }

        return true;
    }

    ae::RegulatoryDevelopment makeDevelopment()
    {
        ae::RegulatoryDevelopmentConfig config{};

        config.localMaterialInputNodeId =
            1;

        config.resourceInputNodeId =
            2;

        config.depositionOutputNodeId =
            3;

        config.neighborhoodLengthScale =
            1.0;

        config.regulatoryTimeStep =
            0.05;

        config.regulatoryStepsPerDevelopmentStep =
            10;

        config.outputHalfSaturation =
            0.2;

        config.outputCooperativity =
            2.0;

        config.depositionRateScale =
            0.25;

        return ae::RegulatoryDevelopment(
            9,
            9,
            6,
            config
        );
    }

    ae::RegulatoryProgram makeActivatingProgram()
    {
        return ae::RegulatoryProgram(
            {
                // Local-material input.
                {
                    1,
                    0.0,
                    0.0,
                    1.0
                },

            // Resource input.
            {
                2,
                0.0,
                0.0,
                1.0
            },

            // Deposition effector.
            {
                3,
                0.0,
                0.01,
                1.0
            }
            },
        {
            {
                1,
                3,
                20.0,
                0.20,
                2.0
            },

            {
                2,
                3,
                20.0,
                0.50,
                2.0
            }
        }
        );
    }

    ae::RegulatoryProgram makeRepressingProgram()
    {
        return ae::RegulatoryProgram(
            {
                {
                    1,
                    0.0,
                    0.0,
                    1.0
                },

                {
                    2,
                    0.0,
                    0.0,
                    1.0
                },

                {
                    3,
                    0.0,
                    0.01,
                    1.0
                }
            },
        {
            {
                1,
                3,
                0.05,
                0.20,
                2.0
            },

            {
                2,
                3,
                0.05,
                0.50,
                2.0
            }
        }
        );
    }

} // namespace

int main()
{
    try
    {
        const ae::RegulatoryDevelopment development =
            makeDevelopment();

        ae::Environment environment{};

        environment.resourceAvailability =
            1.0;


        // --------------------------------------------------------
        // Test 1:
        // The regulatory network actually causes development.
        // --------------------------------------------------------

        const ae::RegulatoryProgram activatingProgram =
            makeActivatingProgram();

        const ae::Phenotype activated =
            development.develop(
                activatingProgram,
                environment
            );

        require(
            activated.totalMaterial() > 1.0,
            "Activating regulatory program failed to produce growth."
        );


        // --------------------------------------------------------
        // Test 2:
        // Different regulatory wiring under the same physical
        // environment produces a different phenotype.
        // --------------------------------------------------------

        const ae::RegulatoryProgram repressingProgram =
            makeRepressingProgram();

        const ae::Phenotype repressed =
            development.develop(
                repressingProgram,
                environment
            );

        require(
            activated.totalMaterial()
                > repressed.totalMaterial(),
            "Regulatory network architecture did not affect phenotype."
        );


        // --------------------------------------------------------
        // Test 3:
        // The same hereditary program responds differently when
        // physical resource availability changes.
        // --------------------------------------------------------

        ae::Environment noResourceEnvironment =
            environment;

        noResourceEnvironment.resourceAvailability =
            0.0;

        const ae::Phenotype noResource =
            development.develop(
                activatingProgram,
                noResourceEnvironment
            );

        require(
            std::abs(
                noResource.totalMaterial()
                - 1.0
            )
            < 1.0e-12,
            "Development occurred without available resource."
        );

        require(
            activated.totalMaterial()
            > noResource.totalMaterial(),
            "Environmental resource availability failed to affect development."
        );


        // --------------------------------------------------------
        // Test 4:
        // Development is deterministic for identical program and
        // environment.
        // --------------------------------------------------------

        const ae::Phenotype repeat =
            development.develop(
                activatingProgram,
                environment
            );

        require(
            phenotypesEqual(
                activated,
                repeat
            ),
            "Regulatory development is not deterministic."
        );


        // --------------------------------------------------------
        // Test 5:
        // Development remains spatially localized rather than
        // spontaneously creating material everywhere.
        // --------------------------------------------------------

        require(
            activated.totalMaterial()
            < static_cast<double>(
                activated.width()
                * activated.height()
                ),
            "Regulatory development filled the entire domain immediately."
        );


        std::cout
            << "All regulatory development tests passed.\n";

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