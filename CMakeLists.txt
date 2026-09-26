cmake_minimum_required(VERSION 3.25)

project(
    AlienEvolution
    VERSION 0.1.0
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
    PRIVATE
        AlienEvolutionCore
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
    PRIVATE
        AlienEvolutionCore
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
        PRIVATE
            AlienEvolutionCore
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
        PRIVATE
            AlienEvolutionCore
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
        PRIVATE
            AlienEvolutionCore
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
        PRIVATE
            AlienEvolutionCore
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
        PRIVATE
            AlienEvolutionCore
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
        PRIVATE
            AlienEvolutionCore
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
        PRIVATE
            AlienEvolutionCore
    )

    add_test(
        NAME SimulationTests
        COMMAND AlienEvolutionSimulationTests
    )

endif()