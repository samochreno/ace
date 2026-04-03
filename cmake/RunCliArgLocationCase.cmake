if(
    NOT DEFINED ACE_BINARY OR
    NOT DEFINED BUILD_DIR OR
    NOT DEFINED SOURCE_DIR OR
    NOT DEFINED LLVM_BIN_DIR OR
    NOT DEFINED CASE_DIR
)
    message(FATAL_ERROR "RunCliArgLocationCase.cmake requires ACE_BINARY, BUILD_DIR, SOURCE_DIR, LLVM_BIN_DIR, and CASE_DIR.")
endif()

function(ace_read_expectation_lines file_path output_var)
    if(NOT EXISTS "${file_path}")
        set(${output_var} "" PARENT_SCOPE)
        return()
    endif()

    file(STRINGS "${file_path}" raw_lines ENCODING UTF-8)
    set(lines "")
    foreach(line IN LISTS raw_lines)
        string(STRIP "${line}" stripped_line)
        if(stripped_line STREQUAL "")
            continue()
        endif()
        if(stripped_line MATCHES "^#")
            continue()
        endif()
        list(APPEND lines "${line}")
    endforeach()

    set(${output_var} ${lines} PARENT_SCOPE)
endfunction()

function(ace_assert_contains_all haystack description file_path)
    ace_read_expectation_lines("${file_path}" required_lines)
    foreach(required_line IN LISTS required_lines)
        string(FIND "${haystack}" "${required_line}" required_line_index)
        if(required_line_index EQUAL -1)
            message(FATAL_ERROR
                "Missing expected ${description} snippet `${required_line}` from `${file_path}`.\n"
                "Actual content was:\n${haystack}"
            )
        endif()
    endforeach()
endfunction()

function(ace_normalize_newlines input output_var)
    string(REPLACE "\r\n" "\n" normalized "${input}")
    set(${output_var} "${normalized}" PARENT_SCOPE)
endfunction()

set(ACE_TEST_PATH "/usr/bin:/bin:/usr/sbin:/sbin:${LLVM_BIN_DIR}")
set(CASE_EXPECT_DIAGNOSTICS_FILE "${CASE_DIR}/expect.diagnostics")
set(CASE_OUTPUT_DIR "${BUILD_DIR}/cliarg_location")
set(PACKAGE_PATH "${SOURCE_DIR}/example/package.json")

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${BUILD_DIR}" --target ace
    RESULT_VARIABLE build_result
)
if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "Failed to build ace for CLI arg location case.")
endif()

file(REMOVE_RECURSE "${CASE_OUTPUT_DIR}")
file(MAKE_DIRECTORY "${CASE_OUTPUT_DIR}")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
        "PATH=${ACE_TEST_PATH}"
        "${ACE_BINARY}"
        "-o${CASE_OUTPUT_DIR}"
        "${PACKAGE_PATH}"
        "${PACKAGE_PATH}"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE compile_result
    OUTPUT_VARIABLE compile_stdout
    ERROR_VARIABLE compile_stderr
)

ace_normalize_newlines("${compile_stdout}" compile_stdout)
ace_normalize_newlines("${compile_stderr}" compile_stderr)
set(compile_output "${compile_stdout}${compile_stderr}")

if(compile_result EQUAL 0)
    message(FATAL_ERROR "CLI arg location case unexpectedly compiled successfully.\n${compile_output}")
endif()

ace_assert_contains_all(
    "${compile_output}"
    "diagnostic"
    "${CASE_EXPECT_DIAGNOSTICS_FILE}"
)
