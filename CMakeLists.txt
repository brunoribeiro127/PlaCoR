cmake_minimum_required(VERSION 3.12)

project(PlaCoR LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH})

# find dependencies
find_package(Spread 5 REQUIRED)
find_package(SSRCSpread 1.0.15 REQUIRED)
find_package(LibEvent 2 REQUIRED)
find_package(Cereal REQUIRED)
find_package(Boost 1.65 REQUIRED COMPONENTS program_options filesystem)
find_package(LibSSH 0.8 REQUIRED)

# check subdirectories
add_subdirectory(src)
add_subdirectory(tools)
