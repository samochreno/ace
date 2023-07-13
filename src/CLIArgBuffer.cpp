#include "CLIArgBuffer.hpp"

#include <vector>
#include <string>
#include <string_view>
#include <utility>

namespace Ace
{
    CLIArgBuffer::CLIArgBuffer(
        const Compilation* const t_compilation,
        const std::vector<std::string_view>& t_args
    ) : m_Compilation{ t_compilation }
    {
        std::vector<std::pair<size_t, size_t>> argIndexLengthPairs{};
        std::for_each(begin(t_args), end(t_args),
        [&](const std::string_view t_arg)
        {
            const auto index = m_Buffer.size();

            m_Buffer += t_arg;
            m_Buffer += '\n';

            argIndexLengthPairs.emplace_back(index, t_arg.size());
        });

        std::transform(
            begin(argIndexLengthPairs),
            end  (argIndexLengthPairs),
            back_inserter(m_Args),
            [&](const std::pair<size_t, size_t>& t_argIndexLengthPair)
            {
                return std::string_view
                {
                    m_Buffer.data() + t_argIndexLengthPair.first,
                    t_argIndexLengthPair.second,
                };
            }
        );
    }

    auto CLIArgBuffer::GetCompilation() const -> const Compilation*
    {
        return m_Compilation;
    }

    auto CLIArgBuffer::GetBuffer() const -> const std::string&
    {
        return m_Buffer;
    }

    auto CLIArgBuffer::FormatLocation(
        const SourceLocation& t_location
    ) const -> std::string
    {
        return {};
    }

    auto CLIArgBuffer::GetArgs() const -> const std::vector<std::string_view>&
    {
        return m_Args;
    }
}
