#include "CLIArgBuffer.hpp"

#include <vector>
#include <string>
#include <string_view>
#include <utility>

namespace Ace
{
    CLIArgBuffer::CLIArgBuffer(
        const Compilation* const compilation,
        const std::vector<std::string_view>& args
    ) : m_Compilation{ compilation }
    {
        std::vector<std::pair<size_t, size_t>> argIndexLengthPairs{};
        std::for_each(begin(args), end(args),
        [&](const std::string_view arg)
        {
            const auto index = m_Buffer.size();

            m_Buffer += arg;
            m_Buffer += '\n';

            argIndexLengthPairs.emplace_back(index, arg.size());
        });

        std::transform(
            begin(argIndexLengthPairs),
            end  (argIndexLengthPairs),
            back_inserter(m_Args),
            [&](const std::pair<size_t, size_t>& argIndexLengthPair)
            {
                return std::string_view
                {
                    m_Buffer.data() + argIndexLengthPair.first,
                    argIndexLengthPair.second,
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
        const SourceLocation& location
    ) const -> std::string
    {
        return {};
    }

    auto CLIArgBuffer::GetArgs() const -> const std::vector<std::string_view>&
    {
        return m_Args;
    }
}
