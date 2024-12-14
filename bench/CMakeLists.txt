find_package(Catch2 3 QUIET)
if (NOT Catch2_FOUND)
	Include(FetchContent)
	FetchContent_Declare(
		Catch2
		DOWNLOAD_EXTRACT_TIMESTAMP OFF
		URL https://github.com/catchorg/Catch2/archive/refs/tags/${CATCH2_VERSION}.tar.gz
	)
	FetchContent_GetProperties(Catch2)
	if (NOT Catch2_POPULATED)
		set(FETCHCONTENT_QUIET NO)
		FetchContent_MakeAvailable(Catch2)
	endif()
endif()

set(bench_name b)

file(GLOB BENCH_SOURCES ${PROJECT_SOURCE_DIR}/bench/bench_*.cpp)
add_executable(${bench_name} ${BENCH_SOURCES} ${SOURCES})
set_target_properties(${bench_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

target_include_directories(${bench_name}
	PRIVATE ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${bench_name}
	PRIVATE Catch2::Catch2WithMain
	CURL::libcurl
	sqlite3
)
