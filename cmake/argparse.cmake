include(FetchContent)

FetchContent_Declare(
	argparse
	DOWNLOAD_EXTRACT_TIMESTAMP OFF
	URL https://github.com/p-ranav/argparse/archive/refs/tags/${ARGPARSE_VERSION}.tar.gz
	URL_HASH SHA256=d01733552ca4a18ab501ae8b8be878131baa32e89090fafdeef018ebfa4c6e46
)
FetchContent_MakeAvailable(argparse)
