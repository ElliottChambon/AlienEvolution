#include <cmath>
#include <iostream>
#include <stdexcept>

#include "alien_evolution/physics/DiffusiveField2D.hpp"

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


    bool nearlyEqual(
        const double a,
        const double b,
        const double tolerance = 1.0e-10
    )
    {
        return
            std::abs(
                a - b
            )
            <= tolerance;
    }

} // namespace


int main()
{
    try
    {
        // --------------------------------------------------------
        // Test 1:
        // A spatially uniform concentration remains uniform under
        // pure diffusion.
        // --------------------------------------------------------

        ae::DiffusiveField2DConfig uniformConfig{};

        uniformConfig.gridSpacing =
            1.0;

        uniformConfig.diffusionCoefficient =
            0.5;

        uniformConfig.decayRate =
            0.0;


        ae::DiffusiveField2D uniformField(
            7,
            7,
            uniformConfig
        );


        for (
            std::size_t y = 0;
            y < uniformField.height();
            ++y
            )
        {
            for (
                std::size_t x = 0;
                x < uniformField.width();
                ++x
                )
            {
                uniformField.setConcentration(
                    x,
                    y,
                    3.0
                );
            }
        }


        uniformField.step(
            0.1
        );


        for (
            std::size_t y = 0;
            y < uniformField.height();
            ++y
            )
        {
            for (
                std::size_t x = 0;
                x < uniformField.width();
                ++x
                )
            {
                require(
                    nearlyEqual(
                        uniformField.concentrationAt(
                            x,
                            y
                        ),
                        3.0
                    ),
                    "Pure diffusion changed a uniform field."
                );
            }
        }


        // --------------------------------------------------------
        // Test 2:
        // Pure diffusion with no-flux boundaries conserves total
        // amount.
        // --------------------------------------------------------

        ae::DiffusiveField2DConfig conservationConfig{};

        conservationConfig.gridSpacing =
            1.0;

        conservationConfig.diffusionCoefficient =
            0.8;

        conservationConfig.decayRate =
            0.0;


        ae::DiffusiveField2D conservationField(
            11,
            11,
            conservationConfig
        );


        conservationField.setConcentration(
            5,
            5,
            10.0
        );


        const double initialAmount =
            conservationField.totalAmount();


        for (
            int step = 0;
            step < 100;
            ++step
            )
        {
            conservationField.step(
                0.1
            );
        }


        require(
            nearlyEqual(
                conservationField.totalAmount(),
                initialAmount,
                1.0e-9
            ),
            "No-flux diffusion failed to conserve total amount."
        );


        // --------------------------------------------------------
        // Test 3:
        // Diffusion from a central point remains symmetric.
        // --------------------------------------------------------

        require(
            nearlyEqual(
                conservationField.concentrationAt(
                    4,
                    5
                ),
                conservationField.concentrationAt(
                    6,
                    5
                )
            ),
            "Diffusion lost left-right symmetry."
        );

        require(
            nearlyEqual(
                conservationField.concentrationAt(
                    5,
                    4
                ),
                conservationField.concentrationAt(
                    5,
                    6
                )
            ),
            "Diffusion lost vertical symmetry."
        );


        // --------------------------------------------------------
        // Test 4:
        // With D = 0 and no source, first-order decay follows the
        // analytical solution exactly.
        //
        // c(t) = c0 * exp(-k t)
        // --------------------------------------------------------

        ae::DiffusiveField2DConfig decayConfig{};

        decayConfig.gridSpacing =
            1.0;

        decayConfig.diffusionCoefficient =
            0.0;

        decayConfig.decayRate =
            0.5;


        ae::DiffusiveField2D decayField(
            1,
            1,
            decayConfig
        );


        decayField.setConcentration(
            0,
            0,
            2.0
        );


        for (
            int step = 0;
            step < 10;
            ++step
            )
        {
            decayField.step(
                0.2
            );
        }


        const double expectedDecay =
            2.0
            * std::exp(
                -1.0
            );


        require(
            nearlyEqual(
                decayField.concentrationAt(
                    0,
                    0
                ),
                expectedDecay,
                1.0e-12
            ),
            "First-order decay disagrees with analytical solution."
        );


        // --------------------------------------------------------
        // Test 5:
        // Constant source plus decay approaches s/k equilibrium.
        // --------------------------------------------------------

        ae::DiffusiveField2DConfig equilibriumConfig{};

        equilibriumConfig.gridSpacing =
            1.0;

        equilibriumConfig.diffusionCoefficient =
            0.0;

        equilibriumConfig.decayRate =
            0.5;


        ae::DiffusiveField2D equilibriumField(
            1,
            1,
            equilibriumConfig
        );


        equilibriumField.setSourceRate(
            0,
            0,
            2.0
        );


        for (
            int step = 0;
            step < 200;
            ++step
            )
        {
            equilibriumField.step(
                0.1
            );
        }


        require(
            nearlyEqual(
                equilibriumField.concentrationAt(
                    0,
                    0
                ),
                4.0,
                1.0e-3
            ),
            "Source-decay field failed to approach analytical equilibrium."
        );


        // --------------------------------------------------------
        // Test 6:
        // An unstable explicit diffusion time step is rejected
        // rather than silently producing invalid concentrations.
        // --------------------------------------------------------

        ae::DiffusiveField2DConfig unstableConfig{};

        unstableConfig.gridSpacing =
            1.0;

        unstableConfig.diffusionCoefficient =
            1.0;


        ae::DiffusiveField2D unstableField(
            5,
            5,
            unstableConfig
        );


        bool unstableStepRejected =
            false;

        try
        {
            unstableField.step(
                0.3
            );
        }
        catch (const std::invalid_argument&)
        {
            unstableStepRejected =
                true;
        }


        require(
            unstableStepRejected,
            "Unstable explicit diffusion step was not rejected."
        );


        std::cout
            << "All diffusive field tests passed.\n";

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