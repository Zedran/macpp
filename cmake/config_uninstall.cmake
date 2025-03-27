# Based on https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#can-i-do-make-uninstall-with-cmake

if(NOT TARGET uninstall)
    configure_file(
        "${SCRIPTS}/uninstall.cmake.in"
        "${SCRIPTS}/uninstall.cmake"
        IMMEDIATE
        @ONLY
    )
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${SCRIPTS}/uninstall.cmake
    )
endif()
