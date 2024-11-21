cmake_minimum_required(VERSION 3.14)
project(macpp)

option(MAKE_TESTS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD           20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(CMAKE_CXX_FLAGS_DEBUG          "-O3 -ggdb3 -Wall -Wextra -Wpedantic"     )
set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG -Wall -Wextra -Wpedantic"   )
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG -Wall -Wextra -Wpedantic"   )

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CATCH2_VERSION v3.7.1)
set(ARGPARSE_VERSION v3.1)

find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)

include(FetchContent)
FetchContent_Declare(
	argparse
	DOWNLOAD_EXTRACT_TIMESTAMP OFF
	URL https://github.com/p-ranav/argparse/archive/refs/tags/${ARGPARSE_VERSION}.tar.gz
)
FetchContent_MakeAvailable(argparse)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
list(FILTER SOURCES EXCLUDE REGEX "main.cpp")

add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl sqlite3 argparse)

if (MAKE_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()
