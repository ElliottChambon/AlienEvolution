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
)

target_include_directories(
    AlienEvolutionCore
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# ------------------------------------------------------------
# Main executable
# ------------------------------------------------------------

add_executable(AlienEvolution
    src/main.cpp
)

target_link_libraries(
    AlienEvolution
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

endif()