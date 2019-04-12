# vecsum
add_library(vecsum SHARED vecsum.cpp)
target_compile_features(vecsum PUBLIC cxx_std_17)
set_target_properties(vecsum PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(vecsum PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(vecsum PROPERTIES COMPILE_FLAGS -O2)
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
target_link_libraries(
    test
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

# example1
add_library(example1 SHARED example1.cpp)
target_compile_features(example1 PUBLIC cxx_std_17)
set_target_properties(example1 PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(example1 PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(example1 PROPERTIES COMPILE_FLAGS -O2)
target_link_libraries(
    example1
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

install(
    TARGETS
        vecsum
        vecsum_spawn
        test
        example1
    LIBRARY
        DESTINATION examples
)

