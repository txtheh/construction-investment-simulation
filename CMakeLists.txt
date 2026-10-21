cmake_minimum_required(VERSION 3.21)
project(ConstructionInvestmentSimulation VERSION 0.1.0 LANGUAGES CXX)

# backend — раньше, потому что frontend будет линковать megapolis_backend
add_subdirectory(backend)
add_subdirectory(frontend)
