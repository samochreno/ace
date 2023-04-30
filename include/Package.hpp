#pragma once

#include <string>
#include <filesystem>
#include <vector>

#include "Error.hpp"

namespace Ace
{
    struct Package
    {
        struct FileDirectoryPath
        {
            FileDirectoryPath(
                const std::filesystem::path& t_path, 
                const std::optional<std::string>& t_optExtensionFilter, 
                const bool& t_isRecursive
            ) : Path{ t_path }, 
                OptExtensionFilter{ t_optExtensionFilter }, 
                IsRecursive{ t_isRecursive }
            {
            }

            std::filesystem::path Path{};
            std::optional<std::string> OptExtensionFilter{};
            bool IsRecursive{};
        };

        class FileDirectoryPaths
        {
        public:
            FileDirectoryPaths() = default;
            FileDirectoryPaths(
                std::vector<std::filesystem::path>&& t_filePaths,
                std::vector<FileDirectoryPath>&& t_fileDirectoryPaths
            ) : m_FilePaths{ std::move(t_filePaths) },
                m_FileDirectoryPaths{ std::move(t_fileDirectoryPaths) }
            {
            }

            auto CreatePaths() const -> std::vector<std::filesystem::path>;

        private:
            std::vector<std::filesystem::path> m_FilePaths{};
            std::vector<FileDirectoryPath> m_FileDirectoryPaths{};
        };

        static auto Parse(const std::string& t_string) -> Expected<Package>;

        std::string Name{};
        FileDirectoryPaths FilePaths{};
        FileDirectoryPaths DependencyFilePaths{};
    };
}
