cmake_minimum_required(VERSION 3.22.1)
project(ace)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_BUILD_TYPE Debug)

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

find_package(fmt REQUIRED)
message(STATUS "Found fmt ${FMT_VERSION}")

file(GLOB_RECURSE FMT_LIBS ${FMT_LIB_DIR})

add_executable(${PROJECT_NAME} src/main.cpp)
file(GLOB_RECURSE SOURCES src/*.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
target_include_directories(${PROJECT_NAME}
    PUBLIC include
    PRIVATE ${LLVM_INCLUDE_DIRS}
    PRIVATE ${FMT_INC_DIR}
)
target_link_libraries(${PROJECT_NAME}
    fmt
    ${LLVM_LIBS}
    ${FMT_LIBS}
)
