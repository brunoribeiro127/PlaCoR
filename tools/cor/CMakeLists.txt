add_executable(cor cor.cpp)

target_compile_features(cor PUBLIC cxx_std_14)
set_target_properties(cor PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(cor PROPERTIES COMPILE_FLAGS -O2)

target_link_libraries(
    cor
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
        cor
    RUNTIME
        DESTINATION bin
)
