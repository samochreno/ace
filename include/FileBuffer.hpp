#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <filesystem>

#include "SourceBuffer.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class FileBuffer : public virtual ISourceBuffer
    {
    public:
        FileBuffer() = default;
        FileBuffer(const FileBuffer&) = delete;
        FileBuffer(FileBuffer&&) = default;
        ~FileBuffer() = default;

        auto operator=(const FileBuffer&) -> FileBuffer& = delete;
        auto operator=(FileBuffer&&) -> FileBuffer& = default;

        static auto Read(
            const Compilation* const t_compilation,
            const std::filesystem::path& t_path
        ) -> Expected<FileBuffer>;

        auto GetCompilation() const -> const Compilation* final { return m_Compilation; }
        auto GetBuffer() const -> std::string_view final { return m_Buffer; }

        auto FormatLocation(
            const SourceLocation& t_location
        ) const -> std::string final;

        auto GetLines() const -> const std::vector<std::string_view>& { return m_Lines; }

    private:
        FileBuffer(
            const Compilation* const t_compilation,
            const std::filesystem::path& t_path,
            std::string&& t_buffer,
            std::vector<std::string_view>&& t_lines,
            std::vector<std::string_view::const_iterator>&& t_lineBeginIterators
        );

        auto FindLineIndex(
            const std::string_view::const_iterator& t_characterIt
        ) const -> size_t;
        auto FindCharacterIndex(
            const size_t& t_lineIndex,
            const std::string_view::const_iterator& t_characterIt
        ) const -> size_t;

        const Compilation* m_Compilation{};
        std::filesystem::path m_Path{};
        std::string m_Buffer{};
        std::vector<std::string_view> m_Lines{};
        std::vector<std::string_view::const_iterator> m_LineBeginIterators{};
    };
}