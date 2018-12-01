install(
    DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/cor"
    DESTINATION
        include
    FILES_MATCHING
        PATTERN "*.[ht]pp"
)
