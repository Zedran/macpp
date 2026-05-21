set(WARN   "-Wall -Wconversion -Wextra -Wpedantic")
set(HARDEN "-fstack-protector-strong -fcf-protection=full -fstack-clash-protection -D_FORTIFY_SOURCE=2 -fPIE -pie")
set(INSTR  "-fsanitize=address,undefined")

set(CMAKE_C_FLAGS_DEBUG            "-Og -ggdb3 ${INSTR}"              )
set(CMAKE_C_FLAGS_RELEASE          "-O3 -DNDEBUG ${HARDEN}"           )
set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g -DNDEBUG ${HARDEN}"        )
set(CMAKE_C_FLAGS_MINSIZEREL       "-Os -DNDEBUG ${HARDEN}"           )

set(CMAKE_CXX_FLAGS_DEBUG          "-Og -ggdb3 ${WARN} ${INSTR}"      )
set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG ${WARN} ${HARDEN}"   )
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG ${WARN} ${HARDEN}")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG ${WARN} ${HARDEN}"   )
