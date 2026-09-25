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

add_executable(AlienEvolution
    src/main.cpp
)

target_include_directories(
    AlienEvolution
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)