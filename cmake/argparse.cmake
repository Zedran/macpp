include(FetchContent)

FetchContent_Declare(
	argparse
	DOWNLOAD_EXTRACT_TIMESTAMP OFF
	URL https://github.com/p-ranav/argparse/archive/refs/tags/${ARGPARSE_VERSION}.tar.gz
)
FetchContent_MakeAvailable(argparse)
