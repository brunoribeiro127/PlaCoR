add_executable(corx corx.cpp)

target_compile_features(corx PUBLIC cxx_std_14)
set_target_properties(corx PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(corx PROPERTIES COMPILE_FLAGS -O2)

target_link_libraries(
    corx
    PRIVATE
        libcor
        Spread
        SSRCSpread
        LibEvent
        Cereal
        LibSSH
        Boost::boost
)

install(
    TARGETS
        corx
    RUNTIME
        DESTINATION bin
)
