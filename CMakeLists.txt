cmake_minimum_required(VERSION 3.25)

project(
    AlienEvolution
    VERSION 0.2.0
    DESCRIPTION "Computational framework for exploring evolutionary outcomes under planetary constraints"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# Core simulation library
# ------------------------------------------------------------

add_library(AlienEvolutionCore
    src/core/Random.cpp

    src/development/Phenotype.cpp
    src/development/Development.cpp

    src/genetics/Mutation.cpp
    src/genetics/RegulatoryProgram.cpp
    src/genetics/RegulatoryDynamics.cpp
    src/genetics/RegulatoryMutation.cpp
    src/genetics/RegulatoryMutationEvent.cpp
    src/genetics/RegulatoryMutationRateModel.cpp
    src/genetics/RegulatoryMutationSampler.cpp
    src/genetics/RegulatoryMutationTargetSelector.cpp

    src/evolution/Organism.cpp
    src/evolution/Population.cpp
    src/evolution/Reproduction.cpp

    src/evaluation/PhenotypeMetrics.cpp
    src/evaluation/Energetics.cpp
    src/evaluation/Fitness.cpp

    src/simulation/Simulation.cpp
)

target_include_directories(
    AlienEvolutionCore
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# ------------------------------------------------------------
# Main executable
# ------------------------------------------------------------

add_executable(
    AlienEvolution
    src/main.cpp
)

target_link_libraries(
    AlienEvolution
    PRIVATE AlienEvolutionCore
)

# ------------------------------------------------------------
# Scientific experiments
# ------------------------------------------------------------

add_executable(
    AlienEvolutionSelectionValidation
    experiments/selection_validation.cpp
)

target_link_libraries(
    AlienEvolutionSelectionValidation
    PRIVATE AlienEvolutionCore
)

add_executable(
    AlienEvolutionMutationSweep
    experiments/mutation_sweep.cpp
)

target_link_libraries(
    AlienEvolutionMutationSweep
    PRIVATE AlienEvolutionCore
)

add_executable(
    AlienEvolutionLongTermMutation
    experiments/long_term_mutation.cpp
)

target_link_libraries(
    AlienEvolutionLongTermMutation
    PRIVATE AlienEvolutionCore
)

# ------------------------------------------------------------
# Tests
# ------------------------------------------------------------

include(CTest)

if(BUILD_TESTING)

    add_executable(
        AlienEvolutionDevelopmentTests
        tests/development_tests.cpp
    )
    target_link_libraries(
        AlienEvolutionDevelopmentTests
        PRIVATE AlienEvolutionCore
    )
    add_test(
        NAME DevelopmentTests
        COMMAND AlienEvolutionDevelopmentTests
    )

    add_executable(
        AlienEvolutionRandomMutationTests
        tests/random_mutation_tests.cpp
    )
    target_link_libraries(
        AlienEvolutionRandomMutationTests
        PRIVATE AlienEvolutionCore
    )
    add_test(
        NAME RandomMutationTests
        COMMAND AlienEvolutionRandomMutationTests
    )

    add_executable(
        AlienEvolutionPopulationTests
        tests/population_tests.cpp
    )
    target_link_libraries(
        AlienEvolutionPopulationTests
        PRIVATE AlienEvolutionCore
    )
    add_test(
        NAME PopulationTests
        COMMAND AlienEvolutionPopulationTests
    )

    add_executable(
        AlienEvolutionEvaluationTests
        tests/evaluation_tests.cpp
    )
    target_link_libraries(
        AlienEvolutionEvaluationTests
        PRIVATE AlienEvolutionCore
    )
    add_test(
        NAME EvaluationTests
        COMMAND AlienEvolutionEvaluationTests
    )

    add_executable(
        AlienEvolutionFitnessTests
        tests/fitness_tests.cpp
    )
    target_link_libraries(
        AlienEvolutionFitnessTests
        PRIVATE AlienEvolutionCore
    )
    add_test(
        NAME FitnessTests
        COMMAND AlienEvolutionFitnessTests
    )

    add_executable(
        AlienEvolutionReproductionTests
        tests/reproduction_tests.cpp
    )
    target_link_libraries(
        AlienEvolutionReproductionTests
        PRIVATE AlienEvolutionCore
    )
    add_test(
        NAME ReproductionTests
        COMMAND AlienEvolutionReproductionTests
    )

    add_executable(
        AlienEvolutionSimulationTests
        tests/simulation_tests.cpp
    )
    target_link_libraries(
        AlienEvolutionSimulationTests
        PRIVATE AlienEvolutionCore
    )
    add_test(
        NAME SimulationTests
        COMMAND AlienEvolutionSimulationTests
    )

    add_executable(
        AlienEvolutionRegulatoryProgramTests
        tests/regulatory_program_tests.cpp
    )
    target_link_libraries(
        AlienEvolutionRegulatoryProgramTests
        PRIVATE AlienEvolutionCore
    )
    add_test(
        NAME RegulatoryProgramTests
        COMMAND AlienEvolutionRegulatoryProgramTests
    )

    add_executable(
        AlienEvolutionRegulatoryDynamicsTests
        tests/regulatory_dynamics_tests.cpp
    )
    target_link_libraries(
        AlienEvolutionRegulatoryDynamicsTests
        PRIVATE AlienEvolutionCore
    )
    add_test(
        NAME RegulatoryDynamicsTests
        COMMAND AlienEvolutionRegulatoryDynamicsTests
    )

    add_executable(
        AlienEvolutionRegulatoryMutationTests
        tests/regulatory_mutation_tests.cpp
    )
    target_link_libraries(
        AlienEvolutionRegulatoryMutationTests
        PRIVATE AlienEvolutionCore
    )
    add_test(
        NAME RegulatoryMutationTests
        COMMAND AlienEvolutionRegulatoryMutationTests
    )

    add_executable(
        AlienEvolutionRegulatoryMutationEventTests
        tests/regulatory_mutation_event_tests.cpp
    )
    target_link_libraries(
        AlienEvolutionRegulatoryMutationEventTests
        PRIVATE AlienEvolutionCore
    )
    add_test(
        NAME RegulatoryMutationEventTests
        COMMAND AlienEvolutionRegulatoryMutationEventTests
    )

    add_executable(
        AlienEvolutionRegulatoryMutationRateModelTests
        tests/regulatory_mutation_rate_model_tests.cpp
    )
    target_link_libraries(
        AlienEvolutionRegulatoryMutationRateModelTests
        PRIVATE AlienEvolutionCore
    )
    add_test(
        NAME RegulatoryMutationRateModelTests
        COMMAND AlienEvolutionRegulatoryMutationRateModelTests
    )

    add_executable(
        AlienEvolutionRegulatoryMutationSamplerTests
        tests/regulatory_mutation_sampler_tests.cpp
    )
    target_link_libraries(
        AlienEvolutionRegulatoryMutationSamplerTests
        PRIVATE AlienEvolutionCore
    )
    add_test(
        NAME RegulatoryMutationSamplerTests
        COMMAND AlienEvolutionRegulatoryMutationSamplerTests
    )

        add_executable(
        AlienEvolutionRegulatoryMutationTargetSelectorTests
        tests/regulatory_mutation_target_selector_tests.cpp
    )

    target_link_libraries(
        AlienEvolutionRegulatoryMutationTargetSelectorTests
        PRIVATE AlienEvolutionCore
    )

    add_test(
        NAME RegulatoryMutationTargetSelectorTests
        COMMAND AlienEvolutionRegulatoryMutationTargetSelectorTests
    )

endif()