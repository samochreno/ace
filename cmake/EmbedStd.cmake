if(NOT DEFINED ACE_STD_INPUT_DIR OR NOT DEFINED ACE_STD_OUTPUT)
    message(FATAL_ERROR "EmbedStd.cmake requires ACE_STD_INPUT_DIR and ACE_STD_OUTPUT.")
endif()

file(GLOB std_sources "${ACE_STD_INPUT_DIR}/*.ace")
list(SORT std_sources)

if(NOT std_sources)
    message(FATAL_ERROR "No standard library sources found in `${ACE_STD_INPUT_DIR}`.")
endif()

get_filename_component(output_directory "${ACE_STD_OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${output_directory}")

file(WRITE "${ACE_STD_OUTPUT}" [=[
#include "Std.hpp"

#include <filesystem>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Compilation.hpp"
#include "FileBuffer.hpp"

namespace
{
    struct EmbeddedSource
    {
        std::filesystem::path Path{};
        std::string_view Buffer{};
    };

]=])

set(source_entries "")
foreach(source_path IN LISTS std_sources)
    get_filename_component(source_name "${source_path}" NAME)
    string(MAKE_C_IDENTIFIER "${source_name}_source" source_identifier)

    file(READ "${source_path}" source_hex HEX)
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "\\\\x\\1" source_bytes "${source_hex}")

    file(APPEND "${ACE_STD_OUTPUT}"
        "    static constexpr char ${source_identifier}[] = \"${source_bytes}\";\n"
    )
    string(APPEND source_entries
        "        { \"std/${source_name}\", { ${source_identifier}, sizeof(${source_identifier}) - 1 } },\n"
    )
endforeach()

file(APPEND "${ACE_STD_OUTPUT}" "\n    static const EmbeddedSource sources[] =\n    {\n${source_entries}    };\n}\n\nnamespace Ace::Std\n{\n    auto GetName() -> const std::string&\n    {\n        static const std::string name{ \"std\" };\n        return name;\n    }\n\n    auto CreateFileBuffers(\n        Compilation* const compilation\n    ) -> std::vector<std::shared_ptr<const FileBuffer>>\n    {\n        std::vector<std::shared_ptr<const FileBuffer>> fileBuffers{};\n        fileBuffers.reserve(std::size(sources));\n\n        for (const auto& source : sources)\n        {\n            fileBuffers.push_back(FileBuffer::Create(\n                compilation,\n                source.Path,\n                source.Buffer\n            ));\n        }\n\n        return fileBuffers;\n    }\n}\n")
