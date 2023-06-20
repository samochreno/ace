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
            const std::vector<std::string>& t_args
        );
        ~CommandLineArgumentBuffer() = default;

        auto operator=(const CommandLineArgumentBuffer&) -> CommandLineArgumentBuffer& = delete;
        auto operator=(CommandLineArgumentBuffer&&) -> CommandLineArgumentBuffer& = default;

        auto GetCompilation() const -> const Compilation* { return m_Compilation; }
        auto GetBuffer() const -> std::string_view final { return m_Buffer; }

        auto FormatLocation(
            const SourceLocation& t_location
        ) const -> std::string final;

        auto GetArgs() const -> const std::vector<std::string_view>& { return m_Args; }

    private:
        const Compilation* m_Compilation{};
        std::string m_Buffer{};
        std::vector<std::string_view> m_Args{};
    };
}
