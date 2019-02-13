# vecsum
add_library(vecsum SHARED vecsum.cpp)
target_compile_features(vecsum PUBLIC cxx_std_14)
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
target_compile_features(vecsum_spawn PUBLIC cxx_std_14)
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

install(
    TARGETS
        fork_join
        vecsum
        vecsum_spawn
    LIBRARY
        DESTINATION examples
)

