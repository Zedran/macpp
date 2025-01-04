set(WARN   "-Wall -Wextra -Wpedantic")
set(HARDEN "-fstack-protector-strong -fcf-protection=full -fstack-clash-protection -D_FORTIFY_SOURCE=2 -fPIE -pie")

set(CMAKE_C_FLAGS_DEBUG            "-O0 -ggdb3 ${WARN}"               )
set(CMAKE_C_FLAGS_RELEASE          "-O3 -DNDEBUG ${WARN} ${HARDEN}"   )
set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g -DNDEBUG ${WARN} ${HARDEN}")
set(CMAKE_C_FLAGS_MINSIZEREL       "-Os -DNDEBUG ${WARN} ${HARDEN}"   )

set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -ggdb3 ${WARN}"               )
set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG ${WARN} ${HARDEN}"   )
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG ${WARN} ${HARDEN}")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG ${WARN} ${HARDEN}"   )
