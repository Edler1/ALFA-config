cmake_minimum_required(VERSION 3.20)
project(GeoCompare)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Build executable in parent dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

find_package(ROOT REQUIRED COMPONENTS Core Geom)
# find_package(DD4hep REQUIRED)


# Define target executable
add_executable(geoCompare
    src/geo_compare.cpp
    src/GeoUtils.cpp
)

target_include_directories(geoCompare PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Define libraries to link
target_link_libraries(geoCompare
    ROOT::Core
    ROOT::Geom
    # DD4hep::DDCore
)

# Define target compilation options
target_compile_options(geoCompare PRIVATE -Wall)




