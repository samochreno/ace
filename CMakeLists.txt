cmake_minimum_required(VERSION 3.22.1)
project(ace)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_BUILD_TYPE Debug)
include(CTest)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

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

add_executable(${PROJECT_NAME} src/main.cpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
target_include_directories(${PROJECT_NAME}
    PUBLIC include
    PRIVATE ${LLVM_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME}
    ${LLVM_LIBS}
)

if(BUILD_TESTING)
    get_filename_component(ACE_LLVM_PREFIX "${LLVM_DIR}/../../.." ABSOLUTE)
    set(ACE_LLVM_BIN_DIR "${ACE_LLVM_PREFIX}/bin")

    add_test(
        NAME smoke_example
        COMMAND ${CMAKE_COMMAND}
            -DACE_BINARY=$<TARGET_FILE:${PROJECT_NAME}>
            -DBUILD_DIR=${CMAKE_BINARY_DIR}
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DLLVM_BIN_DIR=${ACE_LLVM_BIN_DIR}
            -P ${CMAKE_SOURCE_DIR}/cmake/SmokeExample.cmake
    )
endif()
