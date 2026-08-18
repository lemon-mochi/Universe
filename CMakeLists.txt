cmake_minimum_required(VERSION 3.22)

project(UniverseSimulator
        VERSION 1.0
        DESCRIPTION "A simple universe simulator"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX EXTENSIONS OFF)

add_executable(universe_sim
    src/main.cpp
    # src/Universe.cpp
    # src/Star.cpp
    # src/Planet.cpp
    # src/Asteroid.cpp
    # src/Comet.cpp
    # src/BlackHole.cpp
    # src/Galaxy.cpp
    # src/Simulation.cpp
)

target_include_directories(universe_sim PRIVATE include)