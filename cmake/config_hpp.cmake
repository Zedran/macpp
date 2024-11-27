execute_process(
	COMMAND git describe --tags
	OUTPUT_VARIABLE GIT_TAG
	OUTPUT_STRIP_TRAILING_WHITESPACE
	RESULT_VARIABLE TAG_CMD_RESULT
)

if (TAG_CMD_RESULT EQUAL "0")
	execute_process(
		COMMAND git branch --show-current
		OUTPUT_VARIABLE GIT_BRANCH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
else()
	set(GIT_TAG "unknown version")
endif()

configure_file(
	${CONFIG_HPP_TEMPLATE}
	${CONFIG_HPP_OUT}
	@ONLY
)
