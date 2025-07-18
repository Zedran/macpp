cmake_minimum_required(VERSION 3.14)

option(BUNDLE     "Bundle custom build of SQLite3 with the application"           OFF)
option(MAKE_TESTS "Build test and benchmark targets"                              OFF)
option(COVER      "Generate test coverage target if MAKE_TESTS=ON"                OFF)
option(MAKE_MAN   "Generate Linux manual"                                         OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD           20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(SCRIPTS ${CMAKE_SOURCE_DIR}/cmake)

if (LINUX)
    include(${SCRIPTS}/toolchain/gcc.cmake)
elseif (WIN32)
    include(${SCRIPTS}/toolchain/msvc.cmake)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

project(macpp)

set(INC_DIR ${PROJECT_SOURCE_DIR}/include)

set(CONFIG_HPP_TARGET   config_hpp)
set(CONFIG_HPP_SCRIPT   ${SCRIPTS}/config_hpp.cmake)
set(CONFIG_HPP_TEMPLATE ${INC_DIR}/config.hpp.in)
set(CONFIG_HPP_OUT      ${INC_DIR}/config.hpp)

find_package(CURL REQUIRED)
include(${SCRIPTS}/sqlite3.cmake)
include(${SCRIPTS}/argparse.cmake)

include(${SCRIPTS}/manual.cmake)

add_custom_target(${CONFIG_HPP_TARGET}
    COMMAND ${CMAKE_COMMAND}
        -DPROJECT_NAME=${PROJECT_NAME}
        -DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
        -DCONFIG_HPP_TEMPLATE=${CONFIG_HPP_TEMPLATE}
        -DCONFIG_HPP_OUT=${CONFIG_HPP_OUT}
        -P ${CONFIG_HPP_SCRIPT}
)

add_subdirectory(core)

add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${INC_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE core argparse)

add_dependencies(${PROJECT_NAME} ${CONFIG_HPP_TARGET})

include(${SCRIPTS}/install.cmake)
include(${SCRIPTS}/config_uninstall.cmake)

if (MAKE_TESTS)
    include(${SCRIPTS}/catch2.cmake)

    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(bench)
endif()
