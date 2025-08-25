include(${CMAKE_CURRENT_LIST_DIR}/config_uninstall.cmake)

set(DOC_FILES
    ${PROJECT_SOURCE_DIR}/LICENSE
    ${PROJECT_SOURCE_DIR}/NOTICE.md
    ${PROJECT_SOURCE_DIR}/README.md
)

set(BIN_DIR bin)

if (WIN32)
    set(DOC_DIR .)
    install(
        DIRECTORY              "${CMAKE_BINARY_DIR}/$<CONFIG>/"
        DESTINATION            ${BIN_DIR}
        FILES_MATCHING PATTERN "*.dll"
    )
else()
    set(DOC_DIR share/${PROJECT_NAME})
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_DIR})
install(FILES   ${DOC_FILES}    DESTINATION ${DOC_DIR})

if (LINUX AND MAKE_MAN)
    install(FILES ${MAN_GZ_FILE} DESTINATION share/man/man1)
endif()
