cmake_minimum_required(VERSION 3.14)
project(macpp)

option(MAKE_TESTS OFF)
option(COVER OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD           20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -ggdb3 -Wall -Wextra -Wpedantic"     )
set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG -Wall -Wextra -Wpedantic"   )
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG -Wall -Wextra -Wpedantic"   )

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fstack-protector-strong -fcf-protection=full -D_FORTIFY_SOURCE=2")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(SCRIPTS ${PROJECT_SOURCE_DIR}/cmake)

set(CONFIG_HPP_TARGET   config_hpp)
set(CONFIG_HPP_SCRIPT   ${PROJECT_SOURCE_DIR}/cmake/config_hpp.cmake)
set(CONFIG_HPP_TEMPLATE ${PROJECT_SOURCE_DIR}/src/config.hpp.in)
set(CONFIG_HPP_OUT      ${PROJECT_SOURCE_DIR}/src/config.hpp)

set(CATCH2_VERSION v3.7.1)
set(ARGPARSE_VERSION v3.1)

find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)

include(${SCRIPTS}/argparse.cmake)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
list(FILTER SOURCES EXCLUDE REGEX "main.cpp")

add_custom_target(${CONFIG_HPP_TARGET}
	COMMAND ${CMAKE_COMMAND}
		-DPROJECT_NAME=${PROJECT_NAME}
		-DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
		-DCONFIG_HPP_TEMPLATE=${CONFIG_HPP_TEMPLATE}
		-DCONFIG_HPP_OUT=${CONFIG_HPP_OUT}
		-P ${CONFIG_HPP_SCRIPT}
)

add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl sqlite3 argparse)

add_dependencies(${PROJECT_NAME} ${CONFIG_HPP_TARGET})

if (MAKE_TESTS)
	include(${SCRIPTS}/catch2.cmake)

	enable_testing()
	add_subdirectory(tests)
	add_subdirectory(bench)
endif()
