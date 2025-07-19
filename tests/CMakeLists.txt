set(test_name t)

add_executable(${test_name}
    internal/internal.cpp
    test_Conn.cpp
    test_Updater.cpp
    test_Vendor.cpp
    test_utils.cpp
)

set_target_properties(${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
target_include_directories(${test_name} PRIVATE ${INC_DIR} ${PROJECT_SOURCE_DIR}/internal)
target_link_libraries(${test_name} PRIVATE Catch2::Catch2WithMain)
add_sqlite3(${test_name})

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

set(TESTDATA_POISONED_SQL ${TESTDATA_DEST}/poisoned.sql)
set(TESTDATA_POISONED_DB  ${TESTDATA_DEST}/poisoned.db)

add_custom_target(copy_testdata ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${TESTDATA_SRC} ${TESTDATA_DEST}
)

find_program(SQLITE3_EXE sqlite3 REQUIRED)

add_custom_command(
    OUTPUT ${TESTDATA_DB}
    COMMAND ${SQLITE3_EXE} ${TESTDATA_DB} -batch ".read ${TESTDATA_SQL}"
    DEPENDS ${TESTDATA_SQL} copy_testdata
)

add_custom_command(
    OUTPUT ${TESTDATA_POISONED_DB}
    COMMAND ${SQLITE3_EXE} ${TESTDATA_POISONED_DB} -batch ".read ${TESTDATA_POISONED_SQL}"
    DEPENDS ${TESTDATA_POISONED_SQL} copy_testdata
)

add_custom_target(build_db ALL DEPENDS ${TESTDATA_DB} ${TESTDATA_POISONED_DB})

if (COVER)
    target_link_libraries(${test_name} PRIVATE "-Wl,--whole-archive" core_coverage "-Wl,--no-whole-archive")

    target_compile_options(${test_name} PRIVATE -g --coverage)
    target_link_options(${test_name} PRIVATE --coverage)

    find_program(LCOV lcov REQUIRED)
    find_program(GENHTML genhtml REQUIRED)

    add_custom_target(cover
        COMMAND ${LCOV} --quiet --zerocounters --directory ${PROJECT_SOURCE_DIR}
        COMMAND ${test_name}
        COMMAND ${LCOV} --quiet --capture --directory ${PROJECT_SOURCE_DIR} --output-file coverage.info 2> lcov.log
        COMMAND ${LCOV} --quiet --extract coverage.info '${INC_DIR}/*' '${PROJECT_SOURCE_DIR}/core/*' --output-file coverage.info
        COMMAND ${GENHTML} coverage.info --quiet --filter brace --demangle-cpp --dark --output-directory cover
        DEPENDS ${test_name} build_db
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    )
else()
    target_link_libraries(${test_name} PRIVATE core)
endif()
