set(MAN_SRC_FILE ${PROJECT_SOURCE_DIR}/doc/${PROJECT_NAME}.1.md)
set(MAN_OUT_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1)
set(MAN_GZ_FILE  ${MAN_OUT_FILE}.gz)

find_program(PANDOC pandoc REQUIRED)
find_program(GZIP   gzip   REQUIRED)

add_custom_command(
    OUTPUT ${MAN_GZ_FILE}
    COMMAND ${PANDOC} -s -t man ${MAN_SRC_FILE} -o ${MAN_OUT_FILE}
    COMMAND ${GZIP} -f ${MAN_OUT_FILE}
    DEPENDS ${MAN_SRC_FILE}
)

add_custom_target(manual ALL DEPENDS ${MAN_GZ_FILE})
