cmake_minimum_required(VERSION 3.13)
project(libplang
        DESCRIPTION "PLang Markup Language"
        VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")
set(CMAKE_NO_SYSTEM_FROM_IMPORTED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# region antlr
add_definitions(-DANTLR4CPP_STATIC)
# add_subdirectory(${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp ${CMAKE_BINARY_DIR}/libplang/antlr4 EXCLUDE_FROM_ALL)
# list(APPEND CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/cmake)
# set_target_properties(antlr4_static
#         PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
file(GLOB LIBANTLRCPP_SRC
        "${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/*.cpp"
        "${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/atn/*.cpp"
        "${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/dfa/*.cpp"
        "${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/misc/*.cpp"
        "${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/support/*.cpp"
        "${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/tree/*.cpp"
        "${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/tree/pattern/*.cpp"
        "${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/tree/xpath/*.cpp")
add_library(antlr4_static STATIC
        ${LIBANTLRCPP_SRC})
add_library(antlr4::antlr4_static ALIAS antlr4_static)
target_include_directories(antlr4_static
        PRIVATE ${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src
        PRIVATE ${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/atn
        PRIVATE ${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/dfa
        PRIVATE ${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/misc
        PRIVATE ${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/support
        PRIVATE ${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/tree
        PRIVATE ${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/tree/pattern
        PRIVATE ${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src/tree/xpath)
target_link_libraries(antlr4_static
        PRIVATE Threads::Threads)
# endregion

# region Catch2
add_subdirectory(${CMAKE_HOME_DIRECTORY}/deps/Catch2 ${CMAKE_BINARY_DIR}/libplang/Catch2 EXCLUDE_FROM_ALL)
# endregion

# region FTXUI
# set(FTXUI_BUILD_DOCS OFF)
# set(FTXUI_BUILD_EXAMPLES OFF)
# set(FTXUI_BUILD_TESTS OFF)
# set(FTXUI_BUILD_TESTS_FUZZER OFF)
# set(FTXUI_ENABLE_INSTALL ON)
# add_subdirectory(${CMAKE_HOME_DIRECTORY}/deps/FTXUI EXCLUDE_FROM_ALL)
# endregion FTXUI

# region sqlite_orm
# set(SQLITE_ORM_ENABLE_CXX_17 ON)
# add_subdirectory(${CMAKE_HOME_DIRECTORY}/deps/sqlite_orm ${CMAKE_BINARY_DIR}/libplang/sqlite_orm EXCLUDE_FROM_ALL)
find_package(SQLite3 3.38.0 REQUIRED)
# endregion

set(CMAKE_CURRENT_HOME_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CURRENT_INCLUDE_DIR ${CMAKE_CURRENT_HOME_DIRECTORY}/include)
set(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_HOME_DIRECTORY}/src)
set(CMAKE_CURRENT_RESOURCE_DIR ${CMAKE_CURRENT_HOME_DIRECTORY}/res)

compile_resource(PLANG_RESOURCES xxd "${CMAKE_CURRENT_RESOURCE_DIR}/sql")

add_library(plang
        src/base.cpp
        src/corpus.cpp
        src/error.cpp

        src/corpus/path.cpp
        src/corpus/detail.cpp

        src/data/sqlite_undo.c

        src/lang/GENERATED/PlangBaseVisitor.cpp
        src/lang/GENERATED/PlangLexer.cpp
        src/lang/GENERATED/PlangParser.cpp
        src/lang/GENERATED/PlangVisitor.cpp

        src/lang/visitor.cpp

        ${PLANG_RESOURCES})
add_library(plang::libplang ALIAS plang)
target_include_directories(plang
        PUBLIC ${CMAKE_HOME_DIRECTORY}/include
        PRIVATE ${CMAKE_CURRENT_INCLUDE_DIR}
        PRIVATE ${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE ${CMAKE_HOME_DIRECTORY}/deps/sqlite_orm/include)
target_compile_definitions(plang
        PRIVATE -DPLANG_VERSION="${CMAKE_PROJECT_VERSION}"
        PRIVATE -DSQLITE_CORE
        PRIVATE -DSQLITE_ENABLE_UNDO)
target_link_libraries(plang
        PRIVATE Threads::Threads
        PRIVATE antlr4::antlr4_static
        ${SQLite3_LIBRARIES})

# add_library(sqlite_undo SHARED
#         src/data/sqlite_undo.c)
# target_link_libraries(sqlite_undo
#         ${SQLite3_LIBRARIES})

add_executable(test_plang
        test/src/corpus/root/path.cpp)
target_include_directories(test_plang
        PUBLIC ${CMAKE_HOME_DIRECTORY}/include
        PRIVATE ${CMAKE_CURRENT_HOME_DIRECTORY}/test/include
        PRIVATE ${CMAKE_HOME_DIRECTORY}/deps/antlr4/runtime/Cpp/runtime/src)
# target_compile_definitions(test_plang
#         PRIVATE PLANG_TEST_DB="catch.plangdb")
target_link_libraries(test_plang
        PUBLIC plang::libplang
        PRIVATE Catch2::Catch2WithMain)
