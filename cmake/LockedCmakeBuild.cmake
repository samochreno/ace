if(NOT DEFINED ACE_BUILD_DIR)
    message(FATAL_ERROR "LockedCmakeBuild.cmake requires ACE_BUILD_DIR.")
endif()

cmake_path(ABSOLUTE_PATH ACE_BUILD_DIR NORMALIZE OUTPUT_VARIABLE build_dir)
set(lock_path "${build_dir}/ace-build.lock")

file(
    LOCK "${lock_path}"
    GUARD PROCESS
    TIMEOUT 0
    RESULT_VARIABLE lock_result
)
if(NOT lock_result STREQUAL "0")
    message(STATUS "An Ace build is already running in `${build_dir}`. Waiting for it to finish.")
    file(LOCK "${lock_path}" GUARD PROCESS)
endif()

set(build_command "${CMAKE_COMMAND}" --build "${build_dir}")
if(DEFINED ACE_BUILD_TARGET)
    list(APPEND build_command --target "${ACE_BUILD_TARGET}")
endif()
if(DEFINED ACE_BUILD_PARALLEL_LEVEL)
    list(APPEND build_command --parallel "${ACE_BUILD_PARALLEL_LEVEL}")
endif()

execute_process(
    COMMAND ${build_command}
    RESULT_VARIABLE build_result
)
if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "Ace build failed in `${build_dir}`.")
endif()
