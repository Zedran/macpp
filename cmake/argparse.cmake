include(FetchContent)

FetchContent_Declare(
	argparse
	DOWNLOAD_EXTRACT_TIMESTAMP OFF
	GIT_REPOSITORY https://github.com/p-ranav/argparse.git
	GIT_TAG ${ARGPARSE_VERSION}
)
FetchContent_MakeAvailable(argparse)
