#pragma once

#include <vector>
#include <string>
#include <string_view>

#include "SourceBuffer.hpp"

namespace Ace
{
    class CommandLineArgBuffer : public virtual ISourceBuffer
    {
    public:
        CommandLineArgBuffer() = default;
        CommandLineArgBuffer(const CommandLineArgBuffer&) = delete;
        CommandLineArgBuffer(CommandLineArgBuffer&&) = default;
        CommandLineArgBuffer(
            const Compilation* const t_compilation,
            const std::vector<std::string_view>& t_args
        );
        ~CommandLineArgBuffer() = default;

        auto operator=(const CommandLineArgBuffer&) -> CommandLineArgBuffer& = delete;
        auto operator=(CommandLineArgBuffer&&) -> CommandLineArgBuffer& = default;

        auto GetCompilation() const -> const Compilation*;
        auto GetBuffer() const -> const std::string& final;

        auto FormatLocation(
            const SourceLocation& t_location 
        ) const -> std::string final;

        auto GetArgs() const -> const std::vector<std::string_view>&;

    private:
        const Compilation* m_Compilation{};
        std::string m_Buffer{};
        std::vector<std::string_view> m_Args{};
    };
}
