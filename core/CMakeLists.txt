set(CORE_SOURCES
    cache/Conn.cpp
    cache/ConnR.cpp
    cache/ConnRW.cpp
    cache/Stmt.cpp
    cache/StmtPool.cpp
    update/Downloader.cpp
    update/Reader.cpp
    Registry.cpp
    Vendor.cpp
    dir.cpp
    out.cpp
    utils.cpp
)

add_library(core ${CORE_SOURCES})

target_include_directories(core PRIVATE ${INC_DIR})
target_link_libraries(core PRIVATE CURL::libcurl SQLite::SQLite3)

add_dependencies(core config_hpp)

if (COVER)
    add_library(core_coverage ${CORE_SOURCES})

    target_include_directories(core_coverage PRIVATE ${INC_DIR})
    target_link_libraries(core_coverage PRIVATE CURL::libcurl SQLite::SQLite3)

    add_dependencies(core_coverage config_hpp)

    target_compile_options(core_coverage PRIVATE -g --coverage)
    target_link_options(core_coverage PRIVATE --coverage)
endif()
