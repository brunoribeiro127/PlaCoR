add_library(libcor SHARED "")

target_compile_features(libcor PUBLIC cxx_std_14)
set_target_properties(libcor PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(libcor PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(libcor PROPERTIES LIBRARY_OUTPUT_NAME "cor")
set_target_properties(libcor PROPERTIES COMPILE_FLAGS -O2)

target_include_directories(
    libcor
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libcor/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    libcor
    PRIVATE
        Spread
        SSRCSpread
        LibEvent
        Cereal
        LibSSH
)

target_sources(
    libcor
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/cor.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/message.cpp"
)

include(elements/CMakeLists.txt)
include(resources/CMakeLists.txt)
include(services/CMakeLists.txt)
include(system/CMakeLists.txt)
include(utils/CMakeLists.txt)

install(
    TARGETS
        libcor
    LIBRARY
        DESTINATION lib
)
