#pragma once

#include <vector>
#include <string>
#include <string_view>

#include "SourceBuffer.hpp"

namespace Ace
{
    class CLIArgBuffer : public virtual ISourceBuffer
    {
    public:
        CLIArgBuffer() = default;
        CLIArgBuffer(const CLIArgBuffer&) = delete;
        CLIArgBuffer(CLIArgBuffer&&) = default;
        CLIArgBuffer(
            const Compilation* const t_compilation,
            const std::vector<std::string_view>& t_args
        );
        ~CLIArgBuffer() = default;

        auto operator=(const CLIArgBuffer&) -> CLIArgBuffer& = delete;
        auto operator=(CLIArgBuffer&&) -> CLIArgBuffer& = default;

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
