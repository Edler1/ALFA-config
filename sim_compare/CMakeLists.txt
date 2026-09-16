cmake_minimum_required(VERSION 3.20)
project(SimCompare)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Build executable in parent dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

find_package(ROOT REQUIRED COMPONENTS Core Geom)
find_package(EDM4HEP REQUIRED)
find_package(podio REQUIRED)
find_package(DD4hep REQUIRED)


# Define target executable
add_executable(simCompare
    src/sim_compare.cpp
)

target_include_directories(simCompare PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Define libraries to link
target_link_libraries(simCompare
    # ROOT::Core
    # ROOT::Geom
    podio::podio
    EDM4HEP::edm4hep
    podio::podio
    podio::podioRootIO
    podio::podioIO
    ROOT::Core
    ROOT::Hist
    ROOT::MathCore
    ROOT::GenVector
    ROOT::Matrix
    DD4hep::DDCore
)

# Define target compilation options
target_compile_options(simCompare PRIVATE -Wall)




