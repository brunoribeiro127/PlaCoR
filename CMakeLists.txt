cmake_minimum_required(VERSION 3.12)

project(PlaCoR LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH})

if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)

# find dependencies
find_package(Spread 5 REQUIRED)
find_package(SSRCSpread 1.0.15 REQUIRED)
find_package(LibEvent 2 REQUIRED)
find_package(LibEventThreads REQUIRED)
find_package(Cereal REQUIRED)
find_package(LibSSH 0.8 REQUIRED)

find_package(Boost 1.65 REQUIRED COMPONENTS program_options filesystem)

# check subdirectories
add_subdirectory(libcor)
add_subdirectory(tools)
add_subdirectory(examples)
add_subdirectory(tests)
