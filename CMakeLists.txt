cmake_minimum_required(VERSION 3.14)

option(MAKE_TESTS "Build test and benchmark targets"                    OFF)
option(COVER      "Generate test coverage target if MAKE_TESTS=ON"      OFF)
option(MAKE_MAN   "Generate Linux manual"                               OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD           20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS        OFF)

if (LINUX)
    include(TC_GCC)
elseif (WIN32)
    include(TC_MSVC)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

project(macpp)

set(MACPP_CACHE_VERSION 4)

set(INC_DIR ${PROJECT_SOURCE_DIR}/include)

find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)
include(FetchArgparse)

if (MAKE_MAN)
    include(MakeManual)
endif()

add_custom_target(config_hpp
    COMMAND ${CMAKE_COMMAND}
        -DPROJECT_NAME=${PROJECT_NAME}
        -DMACPP_CACHE_VERSION=${MACPP_CACHE_VERSION}
        -DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
        -DINC_DIR=${INC_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/ConfigHPP.cmake"
)

add_subdirectory(core)

add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${INC_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE core argparse CURL::libcurl SQLite::SQLite3)

add_dependencies(${PROJECT_NAME} config_hpp)

include(InstallArtifacts)

if (CPACK_GENERATOR)
    include(GeneratePackages)
endif()

if (MAKE_TESTS)
    include(FetchCatch2)

    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(bench)
endif()
