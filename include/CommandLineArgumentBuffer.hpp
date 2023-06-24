#pragma once

#include <vector>
#include <string>
#include <string_view>

#include "SourceBuffer.hpp"

namespace Ace
{
    class CommandLineArgumentBuffer : public virtual ISourceBuffer
    {
    public:
        CommandLineArgumentBuffer() = default;
        CommandLineArgumentBuffer(const CommandLineArgumentBuffer&) = delete;
        CommandLineArgumentBuffer(CommandLineArgumentBuffer&&) = default;
        CommandLineArgumentBuffer(
            const Compilation* const t_compilation,
            const std::vector<std::string_view>& t_args
        );
        ~CommandLineArgumentBuffer() = default;

        auto operator=(const CommandLineArgumentBuffer&) -> CommandLineArgumentBuffer& = delete;
        auto operator=(CommandLineArgumentBuffer&&) -> CommandLineArgumentBuffer& = default;

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
