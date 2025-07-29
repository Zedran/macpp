execute_process(
    COMMAND           git describe --tags --dirty
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE   CPACK_PACKAGE_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE   GIT_TAG_RESULT
)

if (NOT GIT_TAG_RESULT EQUAL "0")
    message(FATAL_ERROR "Cannot package: Git failed with code ${GIT_TAG_RESULT}")
elseif (CPACK_PACKAGE_VERSION MATCHES "-g")
    message(FATAL_ERROR "Cannot package: untagged commit")
elseif (CPACK_PACKAGE_VERSION MATCHES "dirty")
    message(FATAL_ERROR "Cannot package: dirty working directory")
endif()

string(REGEX REPLACE "^v" "" CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")

set(CPACK_PACKAGE_NAME                "${PROJECT_NAME}"                         )
set(CPACK_PACKAGE_VENDOR              "Wojciech Głąb (github.com/Zedran)"       )
set(CPACK_PACKAGE_CONTACT             "${CPACK_PACKAGE_VENDOR}"                 )
set(CPACK_PACKAGE_HOMEPAGE_URL        "https://github.com/Zedran/macpp"         )
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CLI tool for MAC address lookup."        )

set(CPACK_PACKAGE_FILE_NAME           "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_PACKAGE_CHECKSUM            "SHA256"                                  )

set(CPACK_RESOURCE_FILE_LICENSE       "${PROJECT_SOURCE_DIR}/LICENSE"           )
set(CPACK_RESOURCE_FILE_README        "${PROJECT_SOURCE_DIR}/README.md"         )

set(CPACK_DEBIAN_PACKAGE_SECTION      "utils"                                   )
set(CPACK_DEBIAN_PACKAGE_DEPENDS      "libcurl4, libsqlite3-0"                  )

set(CPACK_RPM_PACKAGE_RELEASE         "1"                                       )
set(CPACK_RPM_PACKAGE_GROUP           "Unspecified"                             )
set(CPACK_RPM_PACKAGE_LICENSE         "MIT"                                     )
set(CPACK_RPM_PACKAGE_REQUIRES        "libcurl, sqlite"                         )

set(CPACK_PACKAGE_INSTALL_DIRECTORY            "${PROJECT_NAME}"                )
set(CPACK_NSIS_DISPLAY_NAME                    "${PROJECT_NAME}"                )
set(CPACK_NSIS_PACKAGE_NAME                    "${PROJECT_NAME}"                )
set(CPACK_NSIS_MODIFY_PATH                     ON                               )
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON                               )

include(CPack)
