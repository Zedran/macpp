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

set(test_name t)
file(GLOB TEST_SOURCES ${PROJECT_SOURCE_DIR}/tests/test_*.cpp)

add_executable(${test_name} ${TEST_SOURCES} ${SOURCES})
set_target_properties(${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

target_include_directories(${test_name}
	PRIVATE ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${test_name}
	PRIVATE Catch2::Catch2WithMain
	CURL::libcurl
)

add_test(
	NAME ${test_name}
	COMMAND $<TARGET_FILE:${test_name}>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)

set_tests_properties(${test_name} PROPERTIES TIMEOUT 30)
