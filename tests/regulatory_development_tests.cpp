#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
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

        for (
            std::size_t y = 0;
            y < a.height();
            ++y
            )
        {
            for (
                std::size_t x = 0;
                x < a.width();
                ++x
                )
            {
                if (
                    std::abs(
                        a.materialAt(
                            x,
                            y
                        )
                        -
                        b.materialAt(
                            x,
                            y
                        )
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


    // Same regulatory information-processing network as the activating
    // program, but with radically different intrinsic kinetics assigned
    // to the two externally imposed physical-input nodes.
    //
    // Under true clamping these differences must have no developmental
    // effect.
    ae::RegulatoryProgram makeAlteredInputKineticsProgram()
    {
        return ae::RegulatoryProgram(
            {
                {
                    1,
                    5.0,
                    25.0,
                    0.2
                },
                {
                    2,
                    8.0,
                    40.0,
                    5.0
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

    template <typename Function>
    void requireInvalid(Function function)
    {
        try
        {
            function();
        }
        catch (const std::invalid_argument&)
        {
            return;
        }
        throw std::runtime_error("Invalid external development inputs were accepted.");
    }

    void testExternalDevelopment()
    {
        const ae::RegulatoryProgram internal({{3, 0.0, 0.01, 1.0}}, {});
        const ae::RegulatoryDevelopmentSignals signals{100, 200};
        const ae::RegulatoryInputInterface inputs(internal, {
            {100, 3, 20.0, 0.20, 2.0}, {200, 3, 20.0, 0.50, 2.0}
        });
        ae::Environment environment{};
        environment.resourceAvailability = 1.0;
        const auto development = makeDevelopment();
        const auto external = development.develop(internal, inputs, signals, environment);
        require(external.totalMaterial() > 1.0, "External-input development did not grow.");
        require(internal.nodeCount() == 1
            && ae::RegulatoryDynamics::initialState(internal).size() == 1,
            "External signals became regulatory nodes/state.");
        require(phenotypesEqual(external,
            development.develop(internal, inputs, signals, environment), 0.0),
            "External-input development is not deterministic.");

        // Only the output node exists. Unused legacy IDs must not be resolved.
        ae::RegulatoryDevelopmentConfig config{};
        config.localMaterialInputNodeId = 999;
        config.resourceInputNodeId = 998;
        config.depositionOutputNodeId = 3;
        config.outputHalfSaturation = 0.2;
        config.depositionRateScale = 0.25;
        const ae::RegulatoryDevelopment oneStep(9, 9, 1, config);
        const auto first = oneStep.develop(internal, inputs, signals, environment);
        require(first.materialAt(4, 4) == 1.0 && first.materialAt(6, 4) == 0.0
            && first.materialAt(0, 0) == 0.0, "Synchronous localized deposition changed.");

        // Independent first-step oracle: axial/diagonal material measurements
        // differ by exp(-1) versus exp(-sqrt(2)) in the eight-neighbor kernel.
        const double axialWeight = std::exp(-1.0);
        const double diagonalWeight = std::exp(-std::sqrt(2.0));
        const double totalWeight = 4.0 * (axialWeight + diagonalWeight);
        for (int diagonal = 0; diagonal < 2; ++diagonal)
        {
            const double local = (diagonal ? diagonalWeight : axialWeight) / totalWeight;
            const double engagement = local * local / (0.04 + local * local);
            const double production = 0.01 * (1.0 + 19.0 * engagement) * 16.2;
            const double output = production * (1.0 - std::exp(-0.5));
            const double expected = 0.25 * output * output / (0.04 + output * output);
            require(std::abs(first.materialAt(5, diagonal ? 5 : 4) - expected) < 1e-8,
                "External development measurement/integration/deposition mismatch.");
        }
        require(first.materialAt(5, 4) > first.materialAt(5, 5),
            "Spatial material signal did not affect development.");

        const ae::RegulatoryInputInterface noLocal(internal, {{200, 3, 20.0, 0.50, 2.0}});
        require(first.totalMaterial()
            > oneStep.develop(internal, noLocal, signals, environment).totalMaterial(),
            "Local sensing channel did not affect development.");
        const ae::RegulatoryInputInterface noResource(internal, {{100, 3, 20.0, 0.20, 2.0}});
        require(first.totalMaterial()
            > oneStep.develop(internal, noResource, signals, environment).totalMaterial(),
            "Resource transduction channel did not affect development independently of deposition.");
        ae::Environment lowResource = environment;
        lowResource.resourceAvailability = 0.25;
        require(external.totalMaterial()
            > development.develop(internal, inputs, signals, lowResource).totalMaterial(),
            "Resource availability did not affect external development.");
        for (double resource : {0.0, -1.0})
        {
            auto absent = environment;
            absent.resourceAvailability = resource;
            require(development.develop(internal, inputs, signals, absent).totalMaterial() == 1.0,
                "Nonpositive resource availability produced growth.");
        }

        // Same response laws and effector kinetics, with dummy clamp nodes
        // removed. Compare the full phenotype across several environments.
        const ae::RegulatoryInputInterface repressive(internal, {
            {100, 3, 0.05, 0.20, 2.0}, {200, 3, 0.05, 0.50, 2.0}
        });
        for (double resource : {0.0, 0.25, 1.0, 2.0})
        {
            auto benchmarkEnvironment = environment;
            benchmarkEnvironment.resourceAvailability = resource;
            require(phenotypesEqual(
                development.develop(makeActivatingProgram(), benchmarkEnvironment),
                development.develop(internal, inputs, signals, benchmarkEnvironment)),
                "Legacy/external activation equivalence benchmark failed.");
            require(phenotypesEqual(
                development.develop(makeRepressingProgram(), benchmarkEnvironment),
                development.develop(internal, repressive, signals, benchmarkEnvironment)),
                "Legacy/external repression equivalence benchmark failed.");
        }

        const ae::RegulatoryInputInterface missing(internal, {{300, 3, 2.0, 1.0, 1.0}});
        requireInvalid([&] { (void)development.develop(internal, missing, signals, environment); });
        const ae::RegulatoryProgram other({{7, 0.0, 0.01, 1.0}}, {});
        const ae::RegulatoryInputInterface mismatched(other, {{100, 7, 2.0, 1.0, 1.0}});
        requireInvalid([&] { (void)development.develop(internal, mismatched, signals, environment); });
        requireInvalid([&] { (void)development.develop(internal, inputs, {100, 100}, environment); });
        config.depositionOutputNodeId = 999;
        const ae::RegulatoryDevelopment badOutput(9, 9, 1, config);
        requireInvalid([&] { (void)badOutput.develop(internal, inputs, signals, environment); });
        config.depositionOutputNodeId = 3;
        const ae::RegulatoryDevelopment zeroSteps(9, 9, 0, config);
        requireInvalid([&] { (void)zeroSteps.develop(internal, missing, signals, environment); });
        require(phenotypesEqual(zeroSteps.develop(internal, inputs, signals, environment),
            zeroSteps.develop(internal, inputs, signals, lowResource)),
            "Zero-step development deposited material.");
        for (double resource : {std::numeric_limits<double>::infinity(),
            std::numeric_limits<double>::quiet_NaN()})
        {
            auto invalidEnvironment = environment;
            invalidEnvironment.resourceAvailability = resource;
            requireInvalid([&] {
                (void)development.develop(internal, inputs, signals, invalidEnvironment);
            });
        }
    }

} // namespace


int main()
{
    try
    {
        testExternalDevelopment();
        const ae::RegulatoryDevelopment development =
            makeDevelopment();

        ae::Environment environment{};

        environment.resourceAvailability =
            1.0;


        // --------------------------------------------------------
        // Test 1:
        // Regulatory network causes development.
        // --------------------------------------------------------

        const ae::RegulatoryProgram activatingProgram =
            makeActivatingProgram();

        const ae::Phenotype activated =
            development.develop(
                activatingProgram,
                environment
            );

        require(
            activated.totalMaterial()
            > 1.0,
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
        // Same hereditary program responds differently to resource
        // availability.
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
        // Development is deterministic.
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
        // Development remains spatially localized.
        // --------------------------------------------------------

        require(
            activated.totalMaterial()
            <
            static_cast<double>(
                activated.width()
                * activated.height()
                ),
            "Regulatory development filled the entire domain immediately."
        );


        // --------------------------------------------------------
        // Test 6:
        // Intrinsic kinetics of externally imposed physical-input
        // nodes must not alter the developed phenotype.
        //
        // This specifically guards against RK4 intermediate-stage
        // leakage from clamped interface nodes.
        // --------------------------------------------------------

        const ae::RegulatoryProgram alteredInputProgram =
            makeAlteredInputKineticsProgram();

        const ae::Phenotype alteredInputPhenotype =
            development.develop(
                alteredInputProgram,
                environment
            );

        require(
            phenotypesEqual(
                activated,
                alteredInputPhenotype
            ),
            "Intrinsic kinetics of externally clamped input nodes altered development."
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
