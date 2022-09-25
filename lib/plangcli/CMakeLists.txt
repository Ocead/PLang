cmake_minimum_required(VERSION 3.13)
project(plangcli
        DESCRIPTION "PLang Console Application"
        VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(PROJECT_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/src)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(Readline REQUIRED)

add_executable(plangcli
        src/cli.cpp
        src/intl.cpp
        src/error.cpp

        src/main.cpp)
add_executable(plang::plangcli ALIAS plangcli)
set_target_properties(plangcli PROPERTIES
        OUTPUT_NAME plang)
target_include_directories(plangcli
        PUBLIC ${PROJECT_INCLUDE_DIR})
target_link_libraries(plangcli
        PRIVATE plang::libplang
        PRIVATE ${Readline_LIBRARIES}
        -Wl,--allow-multiple-definition
        -lreadline
        -lncurses
        -lsqlite3)
