#include "CLIArgBuffer.hpp"

#include <algorithm>
#include <vector>
#include <string>
#include <string_view>
#include <utility>

namespace Ace
{
    CLIArgBuffer::CLIArgBuffer(
        Compilation* const compilation,
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

    auto CLIArgBuffer::GetCompilation() const -> Compilation*
    {
        return m_Compilation;
    }

    auto CLIArgBuffer::GetBuffer() const -> const std::string&
    {
        return m_Buffer;
    }

    auto CLIArgBuffer::FormatLocation(
        const SrcLocation& location
    ) const -> std::string
    {
        const auto lineIt = std::find_if(
            begin(m_Args),
            end  (m_Args),
            [&](const std::string_view arg)
            {
                return
                    (location.CharacterBeginIterator >= begin(arg)) &&
                    (location.CharacterBeginIterator <= end(arg));
            }
        );

        if (lineIt == end(m_Args))
        {
            return "command line";
        }

        const size_t lineIndex = std::distance(begin(m_Args), lineIt);
        const size_t characterIndex = std::distance(
            begin(*lineIt),
            location.CharacterBeginIterator
        );

        return
            "command line:" +
            std::to_string(lineIndex + 1) + ":" +
            std::to_string(characterIndex + 1);
    }

    auto CLIArgBuffer::GetArgs() const -> const std::vector<std::string_view>&
    {
        return m_Args;
    }
}
