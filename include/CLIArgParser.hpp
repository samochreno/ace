#pragma once

#include <vector>
#include <string_view>
#include <optional>
#include <map>

#include "Diagnostics.hpp"
#include "CLIArgBuffer.hpp"

namespace Ace
{
    enum class CLIOptionKind
    {
        WithValue,
        WithoutValue,
    };

    struct CLIOptionDefinition
    {
        std::optional<std::string_view> ShortName{};
        std::string_view LongName{};
        CLIOptionKind Kind{};
        std::optional<std::string> OptDefaultValue{};
    };

    struct CLIOption
    {
        const CLIOptionDefinition* Definition{};
        std::optional<std::string_view> OptValue{};
    };

    struct CLIArgsParseResult
    {
        std::vector<std::string_view> PositionalArgs{};
        std::map<const CLIOptionDefinition*, CLIOption> OptionMap{};
    };

    auto ParseCommandLineArgs(
        const CLIArgBuffer* const t_argBuffer,
        std::vector<const CLIOptionDefinition*>&& t_optionDefinitions
    ) -> Expected<CLIArgsParseResult>;
}
