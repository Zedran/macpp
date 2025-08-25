# Based on https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#can-i-do-make-uninstall-with-cmake

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/Uninstall.cmake.in"
        "${CMAKE_CURRENT_LIST_DIR}/Uninstall.cmake"
        IMMEDIATE
        @ONLY
    )
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/Uninstall.cmake
    )
endif()
