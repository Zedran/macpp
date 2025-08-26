include(FetchContent)

set(CATCH2_VERSION v3.7.1)

FetchContent_Declare(
    Catch2
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/catchorg/Catch2/archive/refs/tags/${CATCH2_VERSION}.tar.gz
    URL_HASH SHA256=c991b247a1a0d7bb9c39aa35faf0fe9e19764213f28ffba3109388e62ee0269c
)
FetchContent_MakeAvailable(Catch2)
