cmake_minimum_required(VERSION 3.13)
project(plang
        DESCRIPTION "PLang Markup Language"
        VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

include(cmake/ResourceCompiler.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_DEBUG_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

add_subdirectory(lib/plang ${CMAKE_BINARY_DIR}/libplang)
add_subdirectory(lib/plangcli ${CMAKE_BINARY_DIR}/plangcli)

find_package(Doxygen)

if (Doxygen_FOUND)
    add_custom_target(plang__doc
            COMMAND ${DOXYGEN_EXECUTABLE} docs/Doxyfile
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            VERBATIM)
endif ()
