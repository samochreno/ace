if(
    NOT DEFINED ACE_BINARY OR
    NOT DEFINED BUILD_DIR OR
    NOT DEFINED SOURCE_DIR OR
    NOT DEFINED LLVM_BIN_DIR
)
    message(FATAL_ERROR "SmokeExample.cmake requires ACE_BINARY, BUILD_DIR, SOURCE_DIR, and LLVM_BIN_DIR.")
endif()

set(ACE_TEST_PATH "/usr/bin:/bin:/usr/sbin:/sbin:${LLVM_BIN_DIR}")
set(EXAMPLE_BUILD_DIR "${SOURCE_DIR}/example/build")

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${BUILD_DIR}" --target ace
    RESULT_VARIABLE build_result
)
if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "Failed to build ace for smoke test.")
endif()

file(REMOVE_RECURSE "${EXAMPLE_BUILD_DIR}")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
        "PATH=${ACE_TEST_PATH}"
        "${ACE_BINARY}" -oexample/build example/package.json
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE compile_result
)
if(NOT compile_result EQUAL 0)
    message(FATAL_ERROR "Failed to compile the example package.")
endif()

execute_process(
    COMMAND "${EXAMPLE_BUILD_DIR}/example"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE run_result
    OUTPUT_VARIABLE run_stdout
    ERROR_VARIABLE run_stderr
)
if(NOT run_result EQUAL 0)
    message(FATAL_ERROR "Example binary failed with exit code ${run_result}: ${run_stderr}")
endif()

if(NOT run_stdout MATCHES "^0(\n|\r\n)?$")
    message(FATAL_ERROR "Unexpected example output: `${run_stdout}`")
endif()
