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
