set(bench_name b)

file(GLOB BENCH_SOURCES ${PROJECT_SOURCE_DIR}/bench/bench_*.cpp)
add_executable(${bench_name} ${BENCH_SOURCES} ${SOURCES})
set_target_properties(${bench_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

target_include_directories(${bench_name}
	PRIVATE ${INC_DIR}
)

target_link_libraries(${bench_name}
	PRIVATE Catch2::Catch2WithMain
	CURL::libcurl
)

add_sqlite3(${bench_name})
