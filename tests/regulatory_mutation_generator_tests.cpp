#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <variant>

#include "alien_evolution/core/Random.hpp"
#include "alien_evolution/genetics/RegulatoryMutationGenerator.hpp"

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
        const double tolerance
    )
    {
        return
            std::abs(a - b)
            <= tolerance;
    }

    template <typename Function>
    void requireThrows(
        Function function,
        const char* message
    )
    {
        bool threw = false;

        try
        {
            function();
        }
        catch (const std::exception&)
        {
            threw = true;
        }

        require(
            threw,
            message
        );
    }

    bool programsEqual(
        const ae::RegulatoryProgram& a,
        const ae::RegulatoryProgram& b
    )
    {
        if (
            a.nodeCount() != b.nodeCount()
            || a.interactionCount()
            != b.interactionCount()
            )
        {
            return false;
        }

        for (std::size_t i = 0;
            i < a.nodeCount();
            ++i)
        {
            const auto& nodeA =
                a.nodes()[i];

            const auto& nodeB =
                b.nodes()[i];

            if (
                nodeA.id != nodeB.id
                || nodeA.initialActivity
                != nodeB.initialActivity
                || nodeA.basalProductionRate
                != nodeB.basalProductionRate
                || nodeA.degradationRate
                != nodeB.degradationRate
                )
            {
                return false;
            }
        }

        for (std::size_t i = 0;
            i < a.interactionCount();
            ++i)
        {
            const auto& edgeA =
                a.interactions()[i];

            const auto& edgeB =
                b.interactions()[i];

            if (
                edgeA.sourceNodeId
                != edgeB.sourceNodeId
                || edgeA.targetNodeId
                != edgeB.targetNodeId
                || edgeA.foldChange
                != edgeB.foldChange
                || edgeA.halfSaturation
                != edgeB.halfSaturation
                || edgeA.cooperativity
                != edgeB.cooperativity
                )
            {
                return false;
            }
        }

        return true;
    }

} // namespace

int main()
{
    try
    {
        const ae::RegulatoryProgram oneNodeProgram(
            {
                {
                    1,
                    0.0,
                    1.0,
                    1.0
                }
            },
            {}
        );

        // --------------------------------------------------------
        // Test 1:
        // Default configuration produces no mutations.
        // --------------------------------------------------------

        ae::RegulatoryMutationGeneratorConfig zeroConfig{};

        ae::Random zeroRandom(12345);

        const auto zeroResult =
            ae::generateRegulatoryOffspring(
                oneNodeProgram,
                zeroConfig,
                zeroRandom
            );

        require(
            zeroResult.events.empty(),
            "Zero-rate mutation generator produced events."
        );

        require(
            programsEqual(
                zeroResult.offspringProgram,
                oneNodeProgram
            ),
            "Zero-rate mutation generator changed offspring."
        );


        // --------------------------------------------------------
        // Test 2:
        // Identical seeds produce identical trajectories.
        // --------------------------------------------------------

        ae::RegulatoryMutationGeneratorConfig deterministicConfig{};

        deterministicConfig.rates.nodeKineticPerNode =
            0.8;

        deterministicConfig.rates.interactionGainPerAbsentPair =
            0.5;

        deterministicConfig.quantitativeEffects
            .nodeBasalProductionLogStdDev =
            0.2;

        deterministicConfig.quantitativeEffects
            .nodeDegradationLogStdDev =
            0.2;

        deterministicConfig.interactionGain
            .foldChangeLogStdDev =
            0.3;

        ae::Random randomA(987654321);
        ae::Random randomB(987654321);

        const auto resultA =
            ae::generateRegulatoryOffspring(
                oneNodeProgram,
                deterministicConfig,
                randomA
            );

        const auto resultB =
            ae::generateRegulatoryOffspring(
                oneNodeProgram,
                deterministicConfig,
                randomB
            );

        require(
            resultA.events.size()
            == resultB.events.size(),
            "Identical seeds produced different event counts."
        );

        for (std::size_t i = 0;
            i < resultA.events.size();
            ++i)
        {
            require(
                resultA.events[i].replicationProgress
                == resultB.events[i].replicationProgress,
                "Identical seeds produced different event timing."
            );

            require(
                resultA.events[i].event.index()
                == resultB.events[i].event.index(),
                "Identical seeds produced different event classes."
            );
        }

        require(
            programsEqual(
                resultA.offspringProgram,
                resultB.offspringProgram
            ),
            "Identical seeds produced different offspring programs."
        );


        // --------------------------------------------------------
        // Test 3:
        // With constant topology and constant hazard, the mean
        // realized event count approaches the Poisson expectation.
        // --------------------------------------------------------

        ae::RegulatoryMutationGeneratorConfig poissonConfig{};

        poissonConfig.rates.nodeKineticPerNode =
            0.8;

        constexpr std::size_t replicateCount =
            20000;

        ae::Random poissonRandom(2222);

        double totalEvents = 0.0;

        for (std::size_t i = 0;
            i < replicateCount;
            ++i)
        {
            const auto result =
                ae::generateRegulatoryOffspring(
                    oneNodeProgram,
                    poissonConfig,
                    poissonRandom
                );

            totalEvents +=
                static_cast<double>(
                    result.events.size()
                    );
        }

        const double meanEvents =
            totalEvents
            / static_cast<double>(
                replicateCount
                );

        require(
            nearlyEqual(
                meanEvents,
                0.8,
                0.025
            ),
            "State-event generator failed constant-hazard Poisson mean check."
        );


        // --------------------------------------------------------
        // Test 4:
        // Structural mutation changes subsequent eligibility.
        //
        // One node with no self-edge has exactly one possible
        // interaction gain.
        //
        // Once that edge is gained, the network is fully connected
        // and the gain hazard becomes zero.
        //
        // Therefore even an enormous gain rate can produce at most
        // one gain event in this state-dependent model.
        // --------------------------------------------------------

        ae::RegulatoryMutationGeneratorConfig gainConfig{};

        gainConfig.rates.interactionGainPerAbsentPair =
            1000.0;

        gainConfig.interactionGain
            .referenceHalfSaturation =
            0.5;

        gainConfig.interactionGain
            .referenceCooperativity =
            2.0;

        ae::Random gainRandom(3333);

        const auto gainResult =
            ae::generateRegulatoryOffspring(
                oneNodeProgram,
                gainConfig,
                gainRandom
            );

        require(
            gainResult.events.size() == 1,
            "Interaction gain eligibility was not recomputed after topology change."
        );

        require(
            gainResult.offspringProgram
            .interactionCount() == 1,
            "Interaction gain did not modify network topology."
        );

        require(
            std::holds_alternative<
            ae::InteractionGainMutationEvent
            >(
                gainResult.events[0].event
            ),
            "Expected interaction-gain mutation event."
        );

        const auto& gainEvent =
            std::get<
            ae::InteractionGainMutationEvent
            >(
                gainResult.events[0].event
            );

        require(
            gainEvent.interaction.sourceNodeId == 1
            && gainEvent.interaction.targetNodeId == 1,
            "Gain event selected incorrect only available target."
        );

        require(
            gainEvent.interaction.foldChange == 1.0,
            "Zero gain-effect scale failed to preserve neutral fold change."
        );

        require(
            gainEvent.interaction.halfSaturation == 0.5,
            "Gain reference half-saturation was not used."
        );

        require(
            gainEvent.interaction.cooperativity == 2.0,
            "Gain reference cooperativity was not used."
        );


        // --------------------------------------------------------
        // Test 5:
        // Every recorded event lies inside the normalized
        // replication interval and events remain ordered.
        // --------------------------------------------------------

        double previousProgress = 0.0;

        for (const auto& timedEvent :
            resultA.events)
        {
            require(
                timedEvent.replicationProgress >= 0.0
                && timedEvent.replicationProgress <= 1.0,
                "Mutation event occurred outside replication interval."
            );

            require(
                timedEvent.replicationProgress
                >= previousProgress,
                "Mutation events are not ordered in replication progress."
            );

            previousProgress =
                timedEvent.replicationProgress;
        }


        // --------------------------------------------------------
        // Test 6:
        // Safety limit throws rather than silently truncating an
        // extreme event process.
        // --------------------------------------------------------

        requireThrows(
            [&oneNodeProgram]
            {
                ae::RegulatoryMutationGeneratorConfig config{};

                config.rates.nodeKineticPerNode =
                    1.0e9;

                config.safetyEventLimit =
                    5;

                ae::Random random(4444);

                const auto result =
                    ae::generateRegulatoryOffspring(
                        oneNodeProgram,
                        config,
                        random
                    );
            },
            "Extreme mutation process did not trigger safety guard."
        );


        std::cout
            << "All regulatory mutation generator tests passed.\n";

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