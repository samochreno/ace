#include "FileBuffer.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>

#include "Diagnostic.hpp"
#include "Diagnostics/FileSystemDiagnostics.hpp"

namespace Ace
{
    auto FileBuffer::Read(
        const Compilation* const t_compilation,
        const std::filesystem::path& t_path
    ) -> Expected<std::shared_ptr<const FileBuffer>>
    {
        DiagnosticBag diagnosticBag{};

        if (!std::filesystem::exists(t_path))
        {
            return diagnosticBag.Add(CreateFileNotFoundError(
                t_path
            ));
        }

        std::ifstream fileStream{ t_path };
        if (!fileStream.is_open())
        {
            return diagnosticBag.Add(CreateFileOpenError(t_path));
        }

        std::string buffer{};
        std::vector<std::pair<size_t, size_t>> lineBeginEndIndexPairs{};

        std::string line{};
        while (std::getline(fileStream, line))
        {
            const auto beginIndex = buffer.size();
            buffer += line;
            const auto endIndex = buffer.size();

            lineBeginEndIndexPairs.emplace_back(beginIndex, endIndex);
            buffer += '\n';
        }

        std::vector<std::string_view> lines{};
        std::transform(
            begin(lineBeginEndIndexPairs),
            end  (lineBeginEndIndexPairs),
            back_inserter(lines),
            [&](const std::pair<size_t, size_t>& t_lineBeginEndIndexPair)
            {
                return std::string_view
                {
                    begin(buffer) + t_lineBeginEndIndexPair.first,
                    begin(buffer) + t_lineBeginEndIndexPair.second,
                };
            }
        );

        std::vector<std::string_view::const_iterator> lineBeginIterators{};
        std::transform(
            begin(lines),
            end  (lines),
            back_inserter(lineBeginIterators),
            [&](const std::string_view t_line)
            {
                return begin(t_line);
            }
        );

        return
        {
            std::shared_ptr<const FileBuffer>(new FileBuffer{
                t_compilation,
                t_path,
                std::move(buffer),
                std::move(lines),
                std::move(lineBeginIterators),
            }),
            diagnosticBag,
        };
    }

    auto FileBuffer::GetCompilation() const -> const Compilation*
    {
        return m_Compilation;
    }

    auto FileBuffer::GetBuffer() const -> const std::string&
    {
        return m_Buffer;
    }

    auto FileBuffer::GetPath() const -> const std::filesystem::path&
    {
        return m_Path;
    }

    auto FileBuffer::GetLines() const -> const std::vector<std::string_view>&
    {
        return m_Lines;
    }


    auto FileBuffer::CreateFirstLocation() const -> SourceLocation
    {
        return
        {
            this,
            begin(m_Lines.front()),
            begin(m_Lines.front()) + 1,
        };
    }

    auto FileBuffer::FormatLocation(
        const SourceLocation& t_location
    ) const -> std::string
    {
        const auto lineIndex = FindLineIndex(
            t_location.CharacterBeginIterator
        );

        const auto characterIndex = FindCharacterIndex(
            lineIndex,
            t_location.CharacterBeginIterator
        );

        return
            m_Path.string() + ':' +
            std::to_string(lineIndex + 1) + ':' +
            std::to_string(characterIndex + 1) + ':' + ' ';
    }


    FileBuffer::FileBuffer(
        const Compilation* const t_compilation,
        const std::filesystem::path& t_path,
        std::string&& t_buffer,
        std::vector<std::string_view>&& t_lines,
        std::vector<std::string_view::const_iterator>&& t_lineBeginIterators
    ) : m_Compilation{ t_compilation },
        m_Path{ t_path },
        m_Buffer{ std::move(t_buffer) },
        m_Lines{ std::move(t_lines) },
        m_LineBeginIterators{ std::move(t_lineBeginIterators) }
    {
    }

    auto FileBuffer::FindLineIndex(
        const std::string_view::const_iterator t_characterIt
    ) const -> size_t
    {
        const auto lineIt = std::upper_bound(
            begin(m_LineBeginIterators),
            end  (m_LineBeginIterators),
            t_characterIt
        ) - 1;

        return std::distance(
            begin(m_LineBeginIterators),
            lineIt
        );
    }

    auto FileBuffer::FindCharacterIndex(
        const size_t t_lineIndex,
        const std::string_view::const_iterator t_characterIt
    ) const -> size_t
    {
        return std::distance(
            m_LineBeginIterators.at(t_lineIndex),
            t_characterIt
        );
    }
}
