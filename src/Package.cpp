#include "Package.hpp"

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <iterator>
#include <nlohmann/json.hpp>

#include "Error.hpp"
#include "Scanner.hpp"
#include "Utility.hpp"

namespace Ace
{
    namespace Property
    {
        static const std::string PathMacros             = "path_macros";
        static const std::string FilePaths              = "file_paths";
        static const std::string DependencyFilePaths    = "dependency_file_paths";
        static const std::string Name                   = "name";
        static const std::string Value                  = "value";
    }

    auto Package::FileDirectoryPaths::CreatePaths() const -> std::vector<std::filesystem::path>
    {
        std::vector<std::filesystem::path> paths{};
        paths.insert(end(paths), begin(m_FilePaths), end(m_FilePaths));

        std::for_each(begin(m_FileDirectoryPaths), end(m_FileDirectoryPaths), [&]
        (const FileDirectoryPath& t_fileDirectoryPath)
        {
            if (!std::filesystem::exists(t_fileDirectoryPath.Path))
                return;

            const auto loopBody = [&](const std::filesystem::directory_entry& t_directoryEntry) -> void
            {
                if (!t_directoryEntry.is_regular_file())
                    return;

                const auto& path = t_directoryEntry.path();

                if (t_fileDirectoryPath.OptExtensionFilter.has_value())
                {
                    if (!path.has_extension())
                        return;

                    if (path.extension() != t_fileDirectoryPath.OptExtensionFilter.value())
                        return;
                }

                paths.push_back(path.string());
            };

            if (t_fileDirectoryPath.IsRecursive)
            {
                for (const auto& directoryEntry : std::filesystem::recursive_directory_iterator(t_fileDirectoryPath.Path))
                {
                    loopBody(directoryEntry);
                }
                    
            }
            else
            {
                for (const auto& directoryEntry : std::filesystem::directory_iterator(t_fileDirectoryPath.Path))
                {
                    loopBody(directoryEntry);
                }
            }
        });

        return paths;
    }

    auto Package::Parse(const std::string& t_string) -> Expected<Package>
    {
        try
        {
            ACE_TRY(package, ParseInternal(t_string));
            return std::move(package);
        }
        catch (nlohmann::json::exception&)
        {
            ACE_TRY_UNREACHABLE();
        }
    }

    struct ExpandedLastFilePathPart
    {
        std::optional<std::string> OptPath{};
        std::optional<std::string> OptExtension{};
        bool IsRecursive{};
    };

    static auto ExpandLastFilePathPart(
        const std::string& t_part
    ) -> Expected<ExpandedLastFilePathPart>
    {
        std::string part = t_part;

        TrimRight(part);

        const size_t extensionDotPosition = part.find_first_of('.');
        const auto optExtension = [&]() -> std::optional<std::string>
        {
            const bool hasExtension = extensionDotPosition != std::string::npos;
            if (!hasExtension)
                return {};

            return std::string
            {
                begin(part) + extensionDotPosition,
                end  (part)
            };
        }();

        const std::string beforeExtension
        {
            begin(part),
            begin(part) + extensionDotPosition
        };

        const bool isFileDirectoryPath = beforeExtension[0] == '*';
        if (!isFileDirectoryPath )
        {
            return ExpandedLastFilePathPart
            {
                part,
                optExtension,
                false
            };
        }

        ACE_TRY(isRecursive, [&]() -> Expected<bool>
        {
            if (beforeExtension.size() <= 1)
                return false;

            ACE_TRY_ASSERT(beforeExtension[1] == '*');
            ACE_TRY_ASSERT(beforeExtension.size() == 2);
            return true;
        }());

        return ExpandedLastFilePathPart
        {
            std::nullopt,
            optExtension,
            isRecursive
        };
    }

    class FileOrDirectoryPath
    {
    public:
        FileOrDirectoryPath() = default;
        FileOrDirectoryPath(const std::filesystem::path& t_filePath)
            : m_OptFilePath{ t_filePath }
        {
        }
        FileOrDirectoryPath(const Package::FileDirectoryPath& t_directoryPath)
            : m_OptDirectoryPath{ t_directoryPath }
        {
        }
        ~FileOrDirectoryPath() = default;

        auto IsFilePath()      const -> bool { return m_OptFilePath.has_value(); }
        auto IsDirectoryPath() const -> bool { return m_OptDirectoryPath.has_value(); }

        auto GetFilePath() -> std::filesystem::path& { return m_OptFilePath.value(); }
        auto GetDirectoryPath() -> Package::FileDirectoryPath& { return m_OptDirectoryPath.value(); }

    private:
        std::optional<std::filesystem::path>      m_OptFilePath{};
        std::optional<Package::FileDirectoryPath> m_OptDirectoryPath{};
    };

    static auto ExpandFirstFilePathPart(
        const std::string& t_part,
        const std::unordered_map<std::string, std::string>& t_pathMacroMap
    ) -> Expected<std::string>
    {
        const bool isMacro = t_part.starts_with('$');
        if (!isMacro)
        {
            return t_part;
        }

        const std::string pathMacro = t_part.substr(1);

        const auto foundIt = t_pathMacroMap.find(pathMacro);
        ACE_TRY_ASSERT(foundIt != end(t_pathMacroMap));

        return foundIt->second;
    }

    static auto ExpandFilePathParts(
        const std::vector<std::string>& t_filePathParts,
        const std::unordered_map<std::string, std::string>& t_pathMacroMap
    ) -> Expected<FileOrDirectoryPath>
    {
        ACE_TRY(firstFilePathPart, ExpandFirstFilePathPart(
            t_filePathParts.front(),
            t_pathMacroMap
        ));

        ACE_TRY(lastFilePathPartData, ExpandLastFilePathPart(
            t_filePathParts.back()
        ));

        std::string path = firstFilePathPart + '/';
        std::for_each(
            begin(t_filePathParts) + 1,
            end  (t_filePathParts) - 1,
            [&](const std::string& t_part) { path += t_part + '/'; }
        );

        const bool isFilePath = lastFilePathPartData.OptPath.has_value();
        if (isFilePath)
        {
            path += lastFilePathPartData.OptPath.value();
            return FileOrDirectoryPath{ std::filesystem::path{ path } };
        }

        return FileOrDirectoryPath
        {
            Package::FileDirectoryPath
            {
                path,
                lastFilePathPartData.OptExtension,
                lastFilePathPartData.IsRecursive
            }
        };
    }

    static auto SplitFilePath(
        const std::string& t_filePath
    ) -> Expected<std::vector<std::string>>
    {
        auto isPathSeparator = [](const char& t_char) -> bool
        {
            return (t_char == '/') || (t_char == '\\');
        };

        std::vector<std::string> parts{};

        auto itBegin = begin(t_filePath);
        auto itEnd   = begin(t_filePath);

        char character{};
        char lastCharacter{};

        for (; itEnd != end(t_filePath); [&]() { lastCharacter = character; ++itEnd; }())
        {
            character = *itEnd;

            const bool isPathSeparatorCurrent = isPathSeparator(character);
            const bool isPathSeparatorLast    = isPathSeparator(lastCharacter);

            if (isPathSeparatorCurrent && !isPathSeparatorLast)
            {
                parts.emplace_back(itBegin, itEnd);
            }

            if (isPathSeparatorLast && !isPathSeparatorCurrent)
            {
                itBegin = itEnd;
            }
        }

        ACE_TRY_ASSERT(!isPathSeparator(lastCharacter));
        parts.emplace_back(itBegin, end(t_filePath));

        ACE_TRY_ASSERT(parts.front().size() != 0);
        return parts;
    }

    static auto TransformFilePaths(
        const std::vector<std::string>& t_filePaths,
        const std::unordered_map<std::string, std::string>& t_pathMacroMap
    ) -> Expected<Package::FileDirectoryPaths>
    {
        ACE_TRY(filePathsParts, TransformExpectedVector(t_filePaths, [&]
        (const std::string& t_filePath) -> Expected<std::vector<std::string>>
        {
            return SplitFilePath(t_filePath);
        }));

        std::vector<std::filesystem::path>      finalFilePaths{};
        std::vector<Package::FileDirectoryPath> finalFileDirectoryPaths{};

        ACE_TRY_VOID(TransformExpectedVector(filePathsParts, [&]
        (const std::vector<std::string>& t_filePathParts) -> Expected<void>
        {
            ACE_TRY(fileOrDirectoryPath, ExpandFilePathParts(
                t_filePathParts,
                t_pathMacroMap
            ));

            if (fileOrDirectoryPath.IsFilePath())
            {
                finalFilePaths.push_back(fileOrDirectoryPath.GetFilePath());
            }

            if (fileOrDirectoryPath.IsDirectoryPath())
            {
                finalFileDirectoryPaths.push_back(fileOrDirectoryPath.GetDirectoryPath());
            }

            return ExpectedVoid;
        }));

        return Package::FileDirectoryPaths
        {
            std::move(finalFilePaths),
            std::move(finalFileDirectoryPaths),
        };
    }

    auto Package::ParseInternal(const std::string& t_string) -> Expected<Package>
    {
        const auto package = nlohmann::json::parse(t_string);

        ACE_TRY_ASSERT(package.contains(Property::Name));
        ACE_TRY_ASSERT(package.contains(Property::PathMacros));
        ACE_TRY_ASSERT(package.contains(Property::FilePaths));
        ACE_TRY_ASSERT(package.contains(Property::DependencyFilePaths));
        ACE_TRY_ASSERT(package.size() == 4);

        std::string name = package[Property::Name];
        
        std::unordered_map<std::string, std::string> pathMacroMap{};
        for (const auto& pathMacro : package[Property::PathMacros])
        {
            ACE_TRY_ASSERT(pathMacro.contains(Property::Name));
            ACE_TRY_ASSERT(pathMacro.contains(Property::Value));
            ACE_TRY_ASSERT(pathMacro.size() == 2);

            const std::string value = pathMacro[Property::Name];
            ACE_TRY_ASSERT(value.size() > 0);

            std::string path = pathMacro[Property::Value];
            pathMacroMap[pathMacro[Property::Name]] = std::move(Trim(path));
        }

        std::vector<std::string> originalFilePaths{};
        std::transform(
            begin(package[Property::FilePaths]),
            end  (package[Property::FilePaths]),
            back_inserter(originalFilePaths),
            [](const nlohmann::json& t_filePath) { return t_filePath; }
        );
        ACE_TRY(finalFilePaths, TransformFilePaths(originalFilePaths, pathMacroMap));

        std::vector<std::string> originalDependencyFilePaths{};
        std::transform(
            begin(package[Property::DependencyFilePaths]),
            end  (package[Property::DependencyFilePaths]),
            back_inserter(originalDependencyFilePaths),
            [](const nlohmann::json& t_filePath) { return t_filePath; }
        );
        ACE_TRY(finalDependencyFilePaths, TransformFilePaths(originalDependencyFilePaths, pathMacroMap));

        return Package
        {
            std::move(name),
            std::move(finalFilePaths),
            std::move(finalDependencyFilePaths),
        };
    }
}
