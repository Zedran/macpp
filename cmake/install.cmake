set(DOC_FILES
    ${PROJECT_SOURCE_DIR}/NOTICE.md
    ${PROJECT_SOURCE_DIR}/README.md
)

set(BIN_DIR bin)

if (WIN32)
    set(DOC_DIR ${CMAKE_INSTALL_PREFIX})
    file(GLOB DLLS ${PROJECT_BINARY_DIR}/Release/*.dll)
    install(FILES ${DLLS} DESTINATION ${BIN_DIR})
else()
    set(DOC_DIR share/${PROJECT_NAME})
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_DIR})
install(FILES   ${DOC_FILES}    DESTINATION ${DOC_DIR})

if (LINUX AND MAKE_MAN)
    install(FILES ${MAN_GZ_FILE} DESTINATION share/man/man1)
endif()
