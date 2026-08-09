file(GLOB_RECURSE DIAGNOSTIC_SOURCE_FILES
    "${SOURCE_DIR}/src/Diagnostics/*.cpp"
    "${SOURCE_DIR}/src/Diagnoses/*.cpp"
    "${SOURCE_DIR}/include/Diagnostics/*.hpp"
    "${SOURCE_DIR}/include/Diagnoses/*.hpp"
)

set(OFFENDING_FILES "")
foreach(SOURCE_FILE IN LISTS DIAGNOSTIC_SOURCE_FILES)
    file(READ "${SOURCE_FILE}" SOURCE_CONTENTS)
    string(FIND "${SOURCE_CONTENTS}" "CreateSignature(" SIGNATURE_CALL_INDEX)

    if(NOT SIGNATURE_CALL_INDEX EQUAL -1)
        file(RELATIVE_PATH RELATIVE_SOURCE_FILE "${SOURCE_DIR}" "${SOURCE_FILE}")
        list(APPEND OFFENDING_FILES "${RELATIVE_SOURCE_FILE}")
    endif()
endforeach()

if(OFFENDING_FILES)
    list(JOIN OFFENDING_FILES "\n  " FORMATTED_OFFENDING_FILES)
    message(FATAL_ERROR
        "Diagnostics must render symbols with CreateDisplayName(), not CreateSignature().\n"
        "Offending files:\n  ${FORMATTED_OFFENDING_FILES}"
    )
endif()
