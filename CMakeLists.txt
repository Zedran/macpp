cmake_minimum_required(VERSION 3.14)
project(macpp)

option(BUNDLE     "Bundle custom builds of SQLite3 and CURL with the application" OFF)
option(MAKE_TESTS "Build test and benchmark targets"                              OFF)
option(COVER      "Generate test coverage target if MAKE_TESTS=ON"                OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD           20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -ggdb3 -Wall -Wextra -Wpedantic"     )
set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG -Wall -Wextra -Wpedantic"   )
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG -Wall -Wextra -Wpedantic"   )

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fstack-protector-strong -fcf-protection=full -fstack-clash-protection -D_FORTIFY_SOURCE=2")

if (NOT BUNDLE)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fPIE -pie")
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(SCRIPTS ${PROJECT_SOURCE_DIR}/cmake)
set(INC_DIR ${PROJECT_SOURCE_DIR}/include)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

set(CONFIG_HPP_TARGET   config_hpp)
set(CONFIG_HPP_SCRIPT   ${SCRIPTS}/config_hpp.cmake)
set(CONFIG_HPP_TEMPLATE ${INC_DIR}/config.hpp.in)
set(CONFIG_HPP_OUT      ${INC_DIR}/config.hpp)

set(CATCH2_VERSION v3.7.1)
set(CURL_VERSION curl-8_11_1)
set(ARGPARSE_VERSION v3.1)

include(${SCRIPTS}/curl.cmake)
include(${SCRIPTS}/sqlite3.cmake)

include(${SCRIPTS}/argparse.cmake)

file(GLOB SOURCES ${SRC_DIR}/*.cpp)
list(FILTER SOURCES EXCLUDE REGEX "main.cpp")

add_custom_target(${CONFIG_HPP_TARGET}
	COMMAND ${CMAKE_COMMAND}
		-DPROJECT_NAME=${PROJECT_NAME}
		-DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
		-DCONFIG_HPP_TEMPLATE=${CONFIG_HPP_TEMPLATE}
		-DCONFIG_HPP_OUT=${CONFIG_HPP_OUT}
		-P ${CONFIG_HPP_SCRIPT}
)

add_executable(${PROJECT_NAME} ${SRC_DIR}/main.cpp ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${INC_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl sqlite3 argparse)

add_dependencies(${PROJECT_NAME} ${CONFIG_HPP_TARGET})

if (MAKE_TESTS)
	include(${SCRIPTS}/catch2.cmake)

	enable_testing()
	add_subdirectory(tests)
	add_subdirectory(bench)
endif()
