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
	sqlite3
)

add_test(
	NAME ${test_name}
	COMMAND $<TARGET_FILE:${test_name}>
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)

set_tests_properties(${test_name} PROPERTIES TIMEOUT 30)

set(TESTDATA_SRC  ${PROJECT_SOURCE_DIR}/tests/testdata)
set(TESTDATA_DEST ${PROJECT_BINARY_DIR}/testdata)
set(TESTDATA_SQL  ${TESTDATA_DEST}/sample.sql)
set(TESTDATA_DB   ${TESTDATA_DEST}/sample.db)

add_custom_target(copy_testdata ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${TESTDATA_SRC} ${TESTDATA_DEST}
)

add_custom_command(
	OUTPUT ${TESTDATA_DB}
	COMMAND sqlite3 ${TESTDATA_DB} -batch ".read ${TESTDATA_SQL}"
	DEPENDS ${TESTDATA_SQL} copy_testdata
)

add_custom_target(build_db ALL DEPENDS ${TESTDATA_DB})

if (COVER)
	target_compile_options(${test_name} PRIVATE -g --coverage)
	target_link_options(${test_name} PRIVATE --coverage)

	find_program(LCOV lcov REQUIRED)
	find_program(GENHTML genhtml REQUIRED)

	add_custom_target(cover
		COMMAND ${LCOV} --quiet --zerocounters --directory .
		COMMAND ${test_name}
		COMMAND ${LCOV} --quiet --capture --directory . --output-file coverage.info 2> lcov.log
		COMMAND ${LCOV} --quiet --extract coverage.info '*/src/*' --output-file coverage.info
		COMMAND ${LCOV} --quiet --remove coverage.info 'catch2*'  --output-file coverage.info
		COMMAND ${GENHTML} coverage.info --quiet --demangle-cpp --dark --output-directory cover
		DEPENDS ${test_name} build_db
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	)
endif()
