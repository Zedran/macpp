set(WARN   "/W4 /WX- /permissive-")
set(HARDEN "/GS /guard:cf /DYNAMICBASE /NXCOMPAT")

set(CMAKE_C_FLAGS_DEBUG            "/Od /Zi ${WARN}"                   )
set(CMAKE_C_FLAGS_RELEASE          "/O2 /DNDEBUG ${WARN} ${HARDEN}"    )
set(CMAKE_C_FLAGS_RELWITHDEBINFO   "/Ox /Zi /DNDEBUG ${WARN} ${HARDEN}")
set(CMAKE_C_FLAGS_MINSIZEREL       "/O1 /DNDEBUG ${WARN} ${HARDEN}"    )

set(CMAKE_CXX_FLAGS_DEBUG          "/Od /Zi ${WARN}"                   )
set(CMAKE_CXX_FLAGS_RELEASE        "/O2 /DNDEBUG ${WARN} ${HARDEN}"    )
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Ox /Zi /DNDEBUG ${WARN} ${HARDEN}")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "/O1 /DNDEBUG ${WARN} ${HARDEN}"    )

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
