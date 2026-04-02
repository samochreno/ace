if(
    NOT DEFINED ACE_BINARY OR
    NOT DEFINED BUILD_DIR OR
    NOT DEFINED SOURCE_DIR OR
    NOT DEFINED LLVM_BIN_DIR
)
    message(FATAL_ERROR "SmokeExample.cmake requires ACE_BINARY, BUILD_DIR, SOURCE_DIR, and LLVM_BIN_DIR.")
endif()

set(CASE_DIR "${SOURCE_DIR}/example")
set(EXPECT_COMPILE "success")
set(EXPECT_STDOUT "0\n")

include("${SOURCE_DIR}/cmake/RunBehaviorCase.cmake")
