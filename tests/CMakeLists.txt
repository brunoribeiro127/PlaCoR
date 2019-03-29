# module1
add_library(module1 SHARED module1.cpp)
target_compile_features(module1 PUBLIC cxx_std_17)
set_target_properties(module1 PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(module1 PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(module1 PROPERTIES COMPILE_FLAGS -O2)
target_link_libraries(
    module1
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

# module2
add_library(module2 SHARED module2.cpp)
target_compile_features(module2 PUBLIC cxx_std_17)
set_target_properties(module2 PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(module2 PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(module2 PROPERTIES COMPILE_FLAGS -O2)
target_link_libraries(
    module2
    PRIVATE
        libcor
        Spread
        SSRCSpread
        Cereal
)

install(
    TARGETS
        module1
        module2
    LIBRARY
        DESTINATION tests
)
