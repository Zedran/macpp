if (BUNDLE)
	include(FetchContent)
	FetchContent_Declare(
		sqlite3
		DOWNLOAD_EXTRACT_TIMESTAMP OFF
		URL https://www.sqlite.org/2024/sqlite-amalgamation-3470200.zip
		URL_HASH SHA256=aa73d8748095808471deaa8e6f34aa700e37f2f787f4425744f53fdd15a89c40
	)
	FetchContent_MakeAvailable(sqlite3)

	add_library(sqlite3 STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
	target_include_directories(sqlite3 PUBLIC ${sqlite3_SOURCE_DIR})

	target_compile_definitions(sqlite3 PRIVATE
		SQLITE_DQS=0
		SQLITE_THREADSAFE=0
		SQLITE_DEFAULT_MEMSTATUS=0
		SQLITE_DEFAULT_WAL_SYNCHRONOUS=0
		SQLITE_LIKE_DOESNT_MATCH_BLOBS
		SQLITE_MAX_EXPR_DEPTH=0
		SQLITE_OMIT_DECLTYPE
		SQLITE_OMIT_DEPRECATED
		SQLITE_OMIT_PROGRESS_CALLBACK
		SQLITE_OMIT_SHARED_CACHE
		SQLITE_USE_ALLOCA
		SQLITE_OMIT_AUTOINIT
	)
else()
	find_package(SQLite3 REQUIRED)
endif()
