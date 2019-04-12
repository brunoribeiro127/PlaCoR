# test pool
add_library(test_pool SHARED test_pool.cpp)
target_compile_features(test_pool PUBLIC cxx_std_17)
set_target_properties(test_pool PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(test_pool PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(test_pool PROPERTIES COMPILE_FLAGS -O2)
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

install(
    TARGETS
        test_pool
    LIBRARY
        DESTINATION tests
)