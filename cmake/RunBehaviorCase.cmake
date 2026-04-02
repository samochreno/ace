if(
    NOT DEFINED ACE_BINARY OR
    NOT DEFINED BUILD_DIR OR
    NOT DEFINED SOURCE_DIR OR
    NOT DEFINED LLVM_BIN_DIR OR
    NOT DEFINED CASE_DIR
)
    message(FATAL_ERROR "RunBehaviorCase.cmake requires ACE_BINARY, BUILD_DIR, SOURCE_DIR, LLVM_BIN_DIR, and CASE_DIR.")
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

function(ace_read_package_name package_json_path output_var)
    file(READ "${package_json_path}" package_json)
    string(REGEX MATCH "\"name\"[ \t\r\n]*:[ \t\r\n]*\"([^\"]+)\"" _ "${package_json}")
    if(NOT CMAKE_MATCH_1)
        message(FATAL_ERROR "Failed to read package name from `${package_json_path}`.")
    endif()

    set(${output_var} "${CMAKE_MATCH_1}" PARENT_SCOPE)
endfunction()

set(ACE_TEST_PATH "/usr/bin:/bin:/usr/sbin:/sbin:${LLVM_BIN_DIR}")
set(CASE_PACKAGE_PATH "${CASE_DIR}/package.json")

if(NOT EXISTS "${CASE_PACKAGE_PATH}")
    message(FATAL_ERROR "Behavior case `${CASE_DIR}` is missing package.json.")
endif()

file(RELATIVE_PATH CASE_RELATIVE_PATH "${SOURCE_DIR}" "${CASE_DIR}")
set(CASE_OUTPUT_DIR "${BUILD_DIR}/behavior/${CASE_RELATIVE_PATH}")
set(CASE_EXPECT_COMPILE_FILE "${CASE_DIR}/expect.compile")
set(CASE_EXPECT_STDOUT_FILE "${CASE_DIR}/expect.stdout")
set(CASE_EXPECT_DIAGNOSTICS_FILE "${CASE_DIR}/expect.diagnostics")
set(CASE_EXPECT_LL_FILE "${CASE_DIR}/expect.ll")

if(DEFINED EXPECT_COMPILE)
    set(expected_compile "${EXPECT_COMPILE}")
elseif(EXISTS "${CASE_EXPECT_COMPILE_FILE}")
    file(READ "${CASE_EXPECT_COMPILE_FILE}" expected_compile_raw)
    string(STRIP "${expected_compile_raw}" expected_compile)
else()
    message(FATAL_ERROR "Behavior case `${CASE_RELATIVE_PATH}` is missing `${CASE_EXPECT_COMPILE_FILE}`.")
endif()

if(NOT expected_compile STREQUAL "success" AND NOT expected_compile STREQUAL "failure")
    message(FATAL_ERROR "`${CASE_EXPECT_COMPILE_FILE}` must contain `success` or `failure`.")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${BUILD_DIR}" --target ace
    RESULT_VARIABLE build_result
)
if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "Failed to build ace for behavior case `${CASE_RELATIVE_PATH}`.")
endif()

file(REMOVE_RECURSE "${CASE_OUTPUT_DIR}")
file(MAKE_DIRECTORY "${CASE_OUTPUT_DIR}")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
        "PATH=${ACE_TEST_PATH}"
        "${ACE_BINARY}" "-o${CASE_OUTPUT_DIR}" "${CASE_PACKAGE_PATH}"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE compile_result
    OUTPUT_VARIABLE compile_stdout
    ERROR_VARIABLE compile_stderr
)

ace_normalize_newlines("${compile_stdout}" compile_stdout)
ace_normalize_newlines("${compile_stderr}" compile_stderr)
set(compile_output "${compile_stdout}${compile_stderr}")

if(expected_compile STREQUAL "success")
    if(NOT compile_result EQUAL 0)
        message(FATAL_ERROR
            "Behavior case `${CASE_RELATIVE_PATH}` failed to compile.\n${compile_output}"
        )
    endif()
else()
    if(compile_result EQUAL 0)
        message(FATAL_ERROR
            "Behavior case `${CASE_RELATIVE_PATH}` unexpectedly compiled successfully.\n${compile_output}"
        )
    endif()

    ace_assert_contains_all(
        "${compile_output}"
        "diagnostic"
        "${CASE_EXPECT_DIAGNOSTICS_FILE}"
    )

    return()
endif()

ace_read_package_name("${CASE_PACKAGE_PATH}" case_package_name)

if(EXISTS "${CASE_EXPECT_LL_FILE}")
    set(case_ll_path "${CASE_OUTPUT_DIR}/${case_package_name}.ll")
    if(NOT EXISTS "${case_ll_path}")
        message(FATAL_ERROR "Behavior case `${CASE_RELATIVE_PATH}` did not emit `${case_ll_path}`.")
    endif()

    file(READ "${case_ll_path}" case_ll_contents)
    ace_assert_contains_all(
        "${case_ll_contents}"
        "IR"
        "${CASE_EXPECT_LL_FILE}"
    )
endif()

if(EXISTS "${CASE_EXPECT_STDOUT_FILE}")
    set(case_binary_path "${CASE_OUTPUT_DIR}/${case_package_name}")
    if(NOT EXISTS "${case_binary_path}")
        message(FATAL_ERROR "Behavior case `${CASE_RELATIVE_PATH}` did not produce `${case_binary_path}`.")
    endif()

    execute_process(
        COMMAND "${case_binary_path}"
        WORKING_DIRECTORY "${SOURCE_DIR}"
        RESULT_VARIABLE run_result
        OUTPUT_VARIABLE run_stdout
        ERROR_VARIABLE run_stderr
    )
    if(NOT run_result EQUAL 0)
        message(FATAL_ERROR
            "Behavior case `${CASE_RELATIVE_PATH}` failed at runtime with exit code ${run_result}.\n${run_stderr}"
        )
    endif()

    file(READ "${CASE_EXPECT_STDOUT_FILE}" expected_stdout)
    ace_normalize_newlines("${run_stdout}" normalized_run_stdout)
    ace_normalize_newlines("${expected_stdout}" normalized_expected_stdout)
    if(NOT normalized_run_stdout STREQUAL normalized_expected_stdout)
        message(FATAL_ERROR
            "Behavior case `${CASE_RELATIVE_PATH}` produced unexpected stdout.\n"
            "Expected:\n${normalized_expected_stdout}\n"
            "Actual:\n${normalized_run_stdout}"
        )
    endif()
elseif(DEFINED EXPECT_STDOUT)
    set(case_binary_path "${CASE_OUTPUT_DIR}/${case_package_name}")
    if(NOT EXISTS "${case_binary_path}")
        message(FATAL_ERROR "Behavior case `${CASE_RELATIVE_PATH}` did not produce `${case_binary_path}`.")
    endif()

    execute_process(
        COMMAND "${case_binary_path}"
        WORKING_DIRECTORY "${SOURCE_DIR}"
        RESULT_VARIABLE run_result
        OUTPUT_VARIABLE run_stdout
        ERROR_VARIABLE run_stderr
    )
    if(NOT run_result EQUAL 0)
        message(FATAL_ERROR
            "Behavior case `${CASE_RELATIVE_PATH}` failed at runtime with exit code ${run_result}.\n${run_stderr}"
        )
    endif()

    ace_normalize_newlines("${run_stdout}" normalized_run_stdout)
    ace_normalize_newlines("${EXPECT_STDOUT}" normalized_expected_stdout)
    if(NOT normalized_run_stdout STREQUAL normalized_expected_stdout)
        message(FATAL_ERROR
            "Behavior case `${CASE_RELATIVE_PATH}` produced unexpected stdout.\n"
            "Expected:\n${normalized_expected_stdout}\n"
            "Actual:\n${normalized_run_stdout}"
        )
    endif()
endif()
