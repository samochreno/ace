cmake_minimum_required(VERSION 3.22.1)
project(ace)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_BUILD_TYPE Debug)
include(CTest)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

find_program(ACE_CLANG_FORMAT
    NAMES clang-format
    HINTS "${LLVM_TOOLS_BINARY_DIR}"
    NO_DEFAULT_PATH
)

if(ACE_CLANG_FORMAT)
    file(GLOB_RECURSE ACE_FORMAT_FILES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/tests/unit/*.cpp"
    )

    add_custom_target(format
        COMMAND "${ACE_CLANG_FORMAT}" -i ${ACE_FORMAT_FILES}
        COMMENT "Formatting C++ sources"
        VERBATIM
    )
    add_custom_target(check-format
        COMMAND "${ACE_CLANG_FORMAT}" --dry-run --Werror ${ACE_FORMAT_FILES}
        COMMENT "Checking C++ source formatting"
        VERBATIM
    )
else()
    message(STATUS "clang-format not found; format targets are unavailable")
endif()

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(LLVM_LIBS
    support
    core
    passes
    analysis
    bitwriter
    native
)

find_package(termcolor REQUIRED)
message(STATUS "Found termcolor")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
list(FILTER SOURCES EXCLUDE REGEX "/src/main\\.cpp$")

file(GLOB ACE_STD_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/std/*.ace"
)
set(ACE_EMBEDDED_STD_SOURCE
    "${CMAKE_BINARY_DIR}/generated/EmbeddedStd.cpp"
)
add_custom_command(
    OUTPUT "${ACE_EMBEDDED_STD_SOURCE}"
    COMMAND ${CMAKE_COMMAND}
        -DACE_STD_INPUT_DIR=${CMAKE_SOURCE_DIR}/std
        -DACE_STD_OUTPUT=${ACE_EMBEDDED_STD_SOURCE}
        -P ${CMAKE_SOURCE_DIR}/cmake/EmbedStd.cmake
    DEPENDS
        ${ACE_STD_SOURCE_FILES}
        ${CMAKE_SOURCE_DIR}/cmake/EmbedStd.cmake
    VERBATIM
)
list(APPEND SOURCES "${ACE_EMBEDDED_STD_SOURCE}")

add_library(ace_core STATIC ${SOURCES})
target_include_directories(ace_core
    PUBLIC include
    PRIVATE ${LLVM_INCLUDE_DIRS}
)
target_link_libraries(ace_core PUBLIC
    ${LLVM_LIBS}
)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ace_core)

if(BUILD_TESTING)
    get_filename_component(ACE_LLVM_PREFIX "${LLVM_DIR}/../../.." ABSOLUTE)
    set(ACE_LLVM_BIN_DIR "${ACE_LLVM_PREFIX}/bin")

    add_test(
        NAME architecture__diagnostics_use_display_names
        COMMAND ${CMAKE_COMMAND}
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
            -P ${CMAKE_SOURCE_DIR}/cmake/CheckDiagnosticDisplayNames.cmake
    )
    set_tests_properties(
        architecture__diagnostics_use_display_names
        PROPERTIES LABELS "architecture"
    )

    add_test(
        NAME smoke_example
        COMMAND ${CMAKE_COMMAND}
            -DACE_BINARY=$<TARGET_FILE:${PROJECT_NAME}>
            -DBUILD_DIR=${CMAKE_BINARY_DIR}
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DLLVM_BIN_DIR=${ACE_LLVM_BIN_DIR}
            -P ${CMAKE_SOURCE_DIR}/cmake/SmokeExample.cmake
    )

    add_executable(symbol_parent_binding_tests
        tests/unit/SymbolParentBindingTests.cpp
    )
    target_link_libraries(symbol_parent_binding_tests PRIVATE ace_core)
    add_test(
        NAME unit__symbol_parent_binding
        COMMAND symbol_parent_binding_tests
    )

    add_executable(embedded_std_tests
        tests/unit/EmbeddedStdTests.cpp
    )
    target_link_libraries(embedded_std_tests PRIVATE ace_core)
    add_test(
        NAME unit__embedded_std
        COMMAND embedded_std_tests
    )

    add_executable(native_verification_tests
        tests/unit/NativeVerificationTests.cpp
    )
    target_link_libraries(native_verification_tests PRIVATE ace_core)
    add_test(
        NAME unit__native_verification
        COMMAND native_verification_tests
    )

    add_executable(token_and_keyword_tests
        tests/unit/TokenAndKeywordTests.cpp
    )
    target_link_libraries(token_and_keyword_tests PRIVATE ace_core)
    add_test(
        NAME unit__token_and_keyword
        COMMAND token_and_keyword_tests
    )

    file(GLOB_RECURSE ACE_BEHAVIOR_PACKAGES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/tests/*/package.json"
    )
    foreach(ACE_BEHAVIOR_PACKAGE IN LISTS ACE_BEHAVIOR_PACKAGES)
        get_filename_component(ACE_BEHAVIOR_CASE_DIR "${ACE_BEHAVIOR_PACKAGE}" DIRECTORY)
        file(RELATIVE_PATH ACE_BEHAVIOR_CASE_RELATIVE "${CMAKE_SOURCE_DIR}/tests" "${ACE_BEHAVIOR_CASE_DIR}")
        string(REPLACE "/" ";" ACE_BEHAVIOR_CASE_PARTS "${ACE_BEHAVIOR_CASE_RELATIVE}")
        list(GET ACE_BEHAVIOR_CASE_PARTS 0 ACE_BEHAVIOR_AREA)
        string(REPLACE "/" "__" ACE_BEHAVIOR_CASE_NAME "${ACE_BEHAVIOR_CASE_RELATIVE}")

        add_test(
            NAME behavior__${ACE_BEHAVIOR_CASE_NAME}
            COMMAND ${CMAKE_COMMAND}
                -DACE_BINARY=$<TARGET_FILE:${PROJECT_NAME}>
                -DBUILD_DIR=${CMAKE_BINARY_DIR}
                -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
                -DLLVM_BIN_DIR=${ACE_LLVM_BIN_DIR}
                -DCASE_DIR=${ACE_BEHAVIOR_CASE_DIR}
                -P ${CMAKE_SOURCE_DIR}/cmake/RunBehaviorCase.cmake
        )
        set_tests_properties(
            behavior__${ACE_BEHAVIOR_CASE_NAME}
            PROPERTIES
                LABELS "behavior;${ACE_BEHAVIOR_AREA}"
        )
    endforeach()
endif()
