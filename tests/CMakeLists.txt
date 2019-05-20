#[[
# test pool
add_library(test_pool SHARED test_pool.cpp)
target_compile_features(test_pool PUBLIC cxx_std_17)
set_target_properties(test_pool PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(test_pool PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(test_pool PROPERTIES COMPILE_FLAGS -O2)
set_target_properties(test_pool PROPERTIES PREFIX "" SUFFIX "")
target_link_libraries(
    test_pool
    PRIVATE
        libcor
        pool
        Spread
        SSRCSpread
        Cereal
)
target_include_directories(
    test_pool
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# test root
add_library(test_root SHARED test_root.cpp)
target_compile_features(test_root PUBLIC cxx_std_17)
set_target_properties(test_root PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(test_root PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(test_root PROPERTIES COMPILE_FLAGS -O2)
set_target_properties(test_root PROPERTIES PREFIX "" SUFFIX "")
target_link_libraries(
    test_root
    PRIVATE
        ROOT::Core
        ${ROOT_LIBRARIES}
        libcor
        pool
        Spread
        SSRCSpread
        Cereal
)
target_include_directories(
    test_root
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# vecsum
add_library(vecsum SHARED vecsum.cpp)
target_compile_features(vecsum PUBLIC cxx_std_17)
set_target_properties(vecsum PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(vecsum PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(vecsum PROPERTIES COMPILE_FLAGS -O2)
set_target_properties(vecsum PROPERTIES PREFIX "" SUFFIX "")
target_link_libraries(
    vecsum
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

# vecsum_spawn
add_library(vecsum_spawn SHARED vecsum_spawn.cpp)
target_compile_features(vecsum_spawn PUBLIC cxx_std_17)
set_target_properties(vecsum_spawn PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(vecsum_spawn PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(vecsum_spawn PROPERTIES COMPILE_FLAGS -O2)
set_target_properties(vecsum_spawn PROPERTIES PREFIX "" SUFFIX "")
target_link_libraries(
    vecsum_spawn
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

# test
add_library(test SHARED test.cpp)
target_compile_features(test PUBLIC cxx_std_17)
set_target_properties(test PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(test PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(test PROPERTIES COMPILE_FLAGS -O2)
set_target_properties(test PROPERTIES PREFIX "" SUFFIX "")
target_link_libraries(
    test
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

# debug
add_library(debug SHARED debug.cpp)
target_compile_features(debug PUBLIC cxx_std_17)
set_target_properties(debug PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(debug PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(debug PROPERTIES COMPILE_FLAGS -O2)
set_target_properties(debug PROPERTIES PREFIX "" SUFFIX "")
target_link_libraries(
    debug
    PRIVATE
        ROOT::Core
        ${ROOT_LIBRARIES}
        libcor
        pool
        Spread
        SSRCSpread
        Cereal
)
target_include_directories(
    debug
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)


# dummy
add_library(dummy SHARED dummy.cpp)
target_compile_features(dummy PUBLIC cxx_std_17)
set_target_properties(dummy PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(dummy PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(dummy PROPERTIES COMPILE_FLAGS -O2)
set_target_properties(dummy PROPERTIES PREFIX "" SUFFIX "")
target_link_libraries(
    dummy
    PRIVATE
        ROOT::Core
        ${ROOT_LIBRARIES}
        libcor
        pool
        Spread
        SSRCSpread
        Cereal
)
target_include_directories(
    dummy
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# test_comm
add_library(test_comm SHARED test_comm.cpp)
target_compile_features(test_comm PUBLIC cxx_std_17)
set_target_properties(test_comm PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(test_comm PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(test_comm PROPERTIES COMPILE_FLAGS -O2)
set_target_properties(test_comm PROPERTIES PREFIX "" SUFFIX "")
target_link_libraries(
    test_comm
    PRIVATE
        ROOT::Core
        ${ROOT_LIBRARIES}
        libcor
        pool
        Spread
        SSRCSpread
        Cereal
)
target_include_directories(
    test_comm
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)
]]

# test_client
add_library(test_client SHARED test_client.cpp)
target_compile_features(test_client PUBLIC cxx_std_17)
set_target_properties(test_client PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(test_client PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(test_client PROPERTIES COMPILE_FLAGS -O2)
set_target_properties(test_client PROPERTIES PREFIX "" SUFFIX "")
target_link_libraries(
    test_client
    PRIVATE
        ROOT::Core
        ${ROOT_LIBRARIES}
        libcor
        pool
        Spread
        SSRCSpread
        Cereal
)
target_include_directories(
    test_client
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# test_server
add_library(test_server SHARED test_server.cpp)
target_compile_features(test_server PUBLIC cxx_std_17)
set_target_properties(test_server PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(test_server PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(test_server PROPERTIES COMPILE_FLAGS -O2)
set_target_properties(test_server PROPERTIES PREFIX "" SUFFIX "")
target_link_libraries(
    test_server
    PRIVATE
        ROOT::Core
        ${ROOT_LIBRARIES}
        libcor
        pool
        Spread
        SSRCSpread
        Cereal
)
target_include_directories(
    test_server
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

install(
    TARGETS
#[[
        test_pool
        test_root
        vecsum
        vecsum_spawn
        test
        debug
        dummy
        test_comm
]]
        test_client
        test_server
    LIBRARY
        DESTINATION tests
)