set(CORE_SOURCES
    cache/Conn.cpp
    cache/ConnR.cpp
    cache/ConnRW.cpp
    cache/Stmt.cpp
    update/Downloader.cpp
    update/Reader.cpp
    Registry.cpp
    Vendor.cpp
    dir.cpp
    utils.cpp
)

add_library(core ${CORE_SOURCES})

target_include_directories(core PRIVATE ${INC_DIR})
target_link_libraries(core PRIVATE CURL::libcurl)
target_add_sqlite3(core)

if (COVER)
    add_library(core_coverage ${CORE_SOURCES})

    target_include_directories(core_coverage PRIVATE ${INC_DIR})
    target_link_libraries(core_coverage PRIVATE CURL::libcurl)
    target_add_sqlite3(core_coverage)

    target_compile_options(core_coverage PRIVATE -g --coverage)
    target_link_options(core_coverage PRIVATE --coverage)
endif()
