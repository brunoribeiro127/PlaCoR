#[[
# module
add_library(module SHARED module.cpp)
target_compile_features(module PUBLIC cxx_std_17)
set_target_properties(module PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(module PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(module PROPERTIES COMPILE_FLAGS -O2)
target_link_libraries(
    module
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

# basicOperations
add_library(basicOperations SHARED basicOperations.cpp)
target_compile_features(basicOperations PUBLIC cxx_std_17)
set_target_properties(basicOperations PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(basicOperations PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(basicOperations PROPERTIES COMPILE_FLAGS -O2)
target_link_libraries(
    basicOperations
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

# callableModule
add_library(callableModule SHARED callableModule.cpp)
target_compile_features(callableModule PUBLIC cxx_std_17)
set_target_properties(callableModule PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(callableModule PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(callableModule PROPERTIES COMPILE_FLAGS -O2)
target_link_libraries(
    callableModule
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

# parallel
add_library(parallel SHARED parallel.cpp)
target_compile_features(parallel PUBLIC cxx_std_17)
set_target_properties(parallel PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(parallel PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(parallel PROPERTIES COMPILE_FLAGS -O2)
target_link_libraries(
    parallel
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

# spawn
add_library(spawn SHARED spawn.cpp)
target_compile_features(spawn PUBLIC cxx_std_17)
set_target_properties(spawn PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(spawn PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(spawn PROPERTIES COMPILE_FLAGS -O2)
target_link_libraries(
    spawn
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

# client
add_library(client SHARED client.cpp)
target_compile_features(client PUBLIC cxx_std_17)
set_target_properties(client PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(client PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(client PROPERTIES COMPILE_FLAGS -O2)
target_link_libraries(
    client
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

# server
add_library(server SHARED server.cpp)
target_compile_features(server PUBLIC cxx_std_17)
set_target_properties(server PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(server PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(server PROPERTIES COMPILE_FLAGS -O2)
target_link_libraries(
    server
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

install(
    TARGETS
        module
        basicOperations
        callableModule
        parallel
        spawn
        client
        server
    LIBRARY
        DESTINATION examples
)
]]