include(FetchContent)

set(ARGPARSE_VERSION cbd9fd8ed675ed6a2ac1bd7142d318c6ad5d3462)

FetchContent_Declare(
    argparse
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG ${ARGPARSE_VERSION}
)
FetchContent_MakeAvailable(argparse)
