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
        Compilation* const compilation,
        const std::filesystem::path& path
    ) -> Expected<std::shared_ptr<const FileBuffer>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (!std::filesystem::exists(path))
        {
            diagnostics.Add(CreateFileNotFoundError(path));
            return std::move(diagnostics);
        }

        std::ifstream fileStream{ path };
        if (!fileStream.is_open())
        {
            diagnostics.Add(CreateFileOpenError(path));
            return std::move(diagnostics);
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
            [&](const std::pair<size_t, size_t>& lineBeginEndIndexPair)
            {
                return std::string_view
                {
                    begin(buffer) + lineBeginEndIndexPair.first,
                    begin(buffer) + lineBeginEndIndexPair.second,
                };
            }
        );

        return
        {
            std::shared_ptr<const FileBuffer>(new FileBuffer{
                compilation,
                path,
                std::move(buffer),
                std::move(lines),
            }),
            std::move(diagnostics),
        };
    }

    auto FileBuffer::Create(
        Compilation* const compilation,
        const std::string_view string
    ) -> std::shared_ptr<const FileBuffer>
    {
        std::string buffer{ string };

        std::vector<std::string_view> lines{};
        size_t i = 0;

        if (buffer.front() == '\n')
        {
            lines.emplace_back(begin(buffer), begin(buffer));
            i++;
        }

        while (i < buffer.size())
        {
            size_t j = i + 1;
            while ((j < buffer.size()) && (buffer.at(j) != '\n'))
            {
                ++j;
            }

            lines.emplace_back(begin(buffer) + i, begin(buffer) + j);
            i = j + 1;
        }

        return std::shared_ptr<const FileBuffer>(new FileBuffer{
            compilation,
            std::filesystem::path{},
            std::move(buffer),
            std::move(lines),
        });
    }

    auto FileBuffer::GetCompilation() const -> Compilation*
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


    auto FileBuffer::CreateFirstLocation() const -> SrcLocation
    {
        return
        {
            this,
            begin(m_Lines.front()),
            begin(m_Lines.front()) + 1,
        };
    }

    auto FileBuffer::FormatLocation(
        const SrcLocation& location
    ) const -> std::string
    {
        const auto lineIndex = FindLineIndex(
            location.CharacterBeginIterator
        );

        const auto characterIndex = FindCharacterIndex(
            lineIndex,
            location.CharacterBeginIterator
        );

        return
            m_Path.string() + ':' +
            std::to_string(lineIndex + 1) + ':' +
            std::to_string(characterIndex + 1);
    }


    FileBuffer::FileBuffer(
        Compilation* const compilation,
        const std::filesystem::path& path,
        std::string&& buffer,
        std::vector<std::string_view>&& lines
    ) : m_Compilation{ compilation },
        m_Path{ path },
        m_Buffer{ std::move(buffer) },
        m_Lines{ std::move(lines) }
    {
    }

    auto FileBuffer::FindLineIndex(
        const std::string_view::const_iterator characterIt
    ) const -> size_t
    {
        auto lineIt = begin(m_Lines);
        while (characterIt >= end(*lineIt))
        {
            ++lineIt;
        }

        return std::distance(begin(m_Lines), lineIt);
    }

    auto FileBuffer::FindCharacterIndex(
        const size_t lineIndex,
        const std::string_view::const_iterator characterIt
    ) const -> size_t
    {
        return std::distance(
            begin(m_Lines.at(lineIndex)),
            characterIt
        );
    }
}
