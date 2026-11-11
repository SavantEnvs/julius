# CMAKE_PROJECT_INCLUDE hook: runs right after upstream's project() call; defers our target definitions
# to the end of upstream's top-level directory so its source lists (CORE_FILES, ...) are all defined.
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL include "${CMAKE_SOURCE_DIR}/mayhem/fuzz_project.cmake")
