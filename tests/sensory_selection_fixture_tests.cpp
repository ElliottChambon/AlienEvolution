#include <iostream>
#include <stdexcept>

#include "sensory_selection_validation.hpp"

namespace
{
    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    b6::Landscape candidate(double fitness, double gradient)
    {
        return {1.0, fitness, 0.0, 0.0, gradient, 1.0, 0.0, false, b6::gradientSign(gradient), {}, {}};
    }
}

int main()
{
    try
    {
        require(b6::selectEnvironments({candidate(0.0, 99.0), candidate(1.0, -3.0), candidate(1.0, 2.0)})
            == std::vector<std::size_t>({1, 2}), "Viability/primary/opposite environment selection failed.");
        require(b6::selectEnvironments({candidate(1.0, 3.0), candidate(1.0, 3.0)})
            == std::vector<std::size_t>({0}), "Candidate-order tie breaking failed.");
        require(b6::selectEnvironments({candidate(1.0, 0.0), candidate(1.0, 1e-10)}).empty(),
            "Unidentifiable landscape selected for evolution.");
        require(b6::adaptiveShift(-1, -0.5) == 0.5 && b6::adaptiveShift(1, 0.5) == 0.5,
            "Adaptive shift direction incorrect.");
        require(b6::mean({1.0, 2.0, 3.0}) == 2.0 && b6::sd({1.0, 2.0, 3.0}) == 1.0,
            "Descriptive mean/sample SD incorrect.");
        require(b6::quantile({3.0, 1.0, 2.0, 4.0}, 0.5) == 2.5
            && std::abs(b6::quantile({1.0, 2.0, 3.0, 4.0}, 0.1) - 1.3) < 1e-12,
            "Quantile interpolation incorrect.");
        const auto program = b6::founder();
        require(program.regulatoryProgram().nodeCount() == 1 && program.sensoryProgram().channelCount() == 1,
            "Founder has dummy input nodes or extra sensors.");
        const auto fixture = b6::mutationConfig(true);
        const auto hazards = ae::computeRegulatoryMutationHazards(program.regulatoryProgram(), fixture.regulatory.rates);
        require(hazards.totalExpectedEvents == 0.0 && fixture.sensory.rates.channelParameterPerChannel == 0.10
            && fixture.sensory.quantitativeEffects.foldChangeLogStdDev == 0.0
            && fixture.sensory.quantitativeEffects.cooperativityLogStdDev == 0.0
            && fixture.sensory.quantitativeEffects.halfSaturationLogStdDev == 0.15,
            "B6 mutation isolation violated.");
        const auto evaluation = b6::evaluate(program, 1.0);
        const auto repeated = b6::evaluate(program, 1.0);
        require(evaluation.fitness == repeated.fitness && evaluation.fitness > 0.0,
            "Fixture evaluation is not deterministic/viable.");
        for (std::size_t y = 0; y < evaluation.phenotype.height(); ++y)
            for (std::size_t x = 0; x < evaluation.phenotype.width(); ++x)
                require(evaluation.phenotype.materialAt(x, y) == repeated.phenotype.materialAt(x, y),
                    "Phenotype evaluation is not deterministic.");
        const auto adapter = program.sensoryProgram().makeRegulatoryInputInterface(program.regulatoryProgram());
        require(adapter.modulationFactor(3, {{b6::signalIds.resourceSignalId, 0.0}}) == 1.0
            && adapter.modulationFactor(3, {{b6::signalIds.resourceSignalId, 1.0}}) == 10.5,
            "Standardized response assay mismatch.");
        // No stochastic evolution is run by this fast target.
        std::cout << "All sensory selection fixture tests passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
