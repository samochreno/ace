cmake_minimum_required(VERSION 3.22.1)
project(ace)
set(CMAKE_CXX_STANDARD 20)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(${PROJECT_NAME} src/main.cpp)

llvm_map_components_to_libnames(LLVM_LIBS
    support
    core
    passes
    analysis
    bitwriter
    native
)
target_link_libraries(${PROJECT_NAME} ${LLVM_LIBS})

file(GLOB_RECURSE SOURCES src/*.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
target_include_directories(
    ${PROJECT_NAME} PUBLIC include
)
target_link_libraries(${PROJECT_NAME}
    fmt
)
