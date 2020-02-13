cmake_minimum_required(VERSION 2.8.7)

project(cdat CXX C)

set(PROJECT_VENDOR "Michał Sabaciński")

find_package(Boost COMPONENTS program_options)

set(SDSL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals/sdsl-lite)
set(SDSL_INCLUDES ${SDSL_DIR}/include)
set(SDSL_DEPS_INCLUDES ${SDSL_DIR}/external/libdivsufsort-2.0.1/include)
set(INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include)

configure_file (
        "include/config/Config.h.in"
        "include/config/Config.h"
)

option(DEBUG OFF)

set(COMPILE_WARNINGS "-Wall")
add_subdirectory(externals)
add_subdirectory(src)
add_subdirectory(tools)

include_directories(include)