set(bench_name b)

add_executable(${bench_name}
    bench_Vendor.cpp
)

set_target_properties(${bench_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
target_include_directories(${bench_name} PRIVATE ${INC_DIR})
target_link_libraries(${bench_name} PRIVATE Catch2::Catch2WithMain core)
