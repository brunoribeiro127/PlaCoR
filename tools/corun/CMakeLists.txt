add_executable(corun corun.cpp host_file_parser.cpp remote_session.cpp session_manager.cpp)

target_compile_features(corun PUBLIC cxx_std_14)
set_target_properties(corun PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(corun PROPERTIES COMPILE_FLAGS -O2)

target_link_libraries(
    corun
    PRIVATE
        LibSSH
        Boost::program_options
        Boost::filesystem
)

install(
    TARGETS
        corun
    RUNTIME
        DESTINATION bin
)
