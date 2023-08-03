#pragma once

#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>

#include "SrcBuffer.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class FileBuffer : public virtual ISrcBuffer
    {
    public:
        FileBuffer() = default;
        FileBuffer(const FileBuffer&) = delete;
        FileBuffer(FileBuffer&&) = default;
        ~FileBuffer() = default;

        auto operator=(const FileBuffer&) -> FileBuffer& = delete;
        auto operator=(FileBuffer&&) -> FileBuffer& = default;

        static auto Read(
            Compilation* const compilation,
            const std::filesystem::path& path
        ) -> Expected<std::shared_ptr<const FileBuffer>>;

        auto GetCompilation() const -> Compilation* final;
        auto GetBuffer() const -> const std::string& final;

        auto FormatLocation(
            const SrcLocation& location
        ) const -> std::string final;

        auto GetPath() const -> const std::filesystem::path&;
        auto GetLines() const -> const std::vector<std::string_view>&;

        auto CreateFirstLocation() const -> SrcLocation;

    private:
        FileBuffer(
            Compilation* const compilation,
            const std::filesystem::path& path,
            std::string&& buffer,
            std::vector<std::string_view>&& lines,
            std::vector<std::string_view::const_iterator>&& lineBeginIterators
        );

        auto FindLineIndex(
            const std::string_view::const_iterator characterIt
        ) const -> size_t;
        auto FindCharacterIndex(
            const size_t lineIndex,
            const std::string_view::const_iterator characterIt
        ) const -> size_t;

        Compilation* m_Compilation{};
        std::filesystem::path m_Path{};
        std::string m_Buffer{};
        std::vector<std::string_view> m_Lines{};
        std::vector<std::string_view::const_iterator> m_LineBeginIterators{};
    };
}
