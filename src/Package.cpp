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
        static const std::string PathMacros             = "pathMacros";
        static const std::string FilePaths              = "filePaths";
        static const std::string DependencyFilePaths    = "dependencyFilePaths";
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
                    loopBody(directoryEntry);
            }
            else
            {
                for (const auto& directoryEntry : std::filesystem::directory_iterator(t_fileDirectoryPath.Path))
                    loopBody(directoryEntry);
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

    static auto TransformFilePaths(const std::vector<std::string>& t_filePaths, const std::unordered_map<std::string, std::string>& t_pathMacroMap) -> Expected<Package::FileDirectoryPaths>
    {
        ACE_TRY(filePathsParts, TransformExpectedVector(t_filePaths, [&]
        (const std::string& t_filePath) -> Expected<std::vector<std::string>>
        {
            auto isPathSeparator = [](const char& t_char) -> bool
            {
                return (t_char == '/') || (t_char == '\\');
            };

            const std::string path = t_filePath;
            std::vector<std::string> parts{};

            auto itBegin = begin(path);
            auto itEnd = begin(path);

            char character{};
            char lastCharacter{};

            for (; itEnd != end(path); [&]() { lastCharacter = character; ++itEnd; }())
            {
                character = *itEnd;

                const bool isPathSeparatorCurrent = isPathSeparator(character);
                const bool isPathSeparatorLast = isPathSeparator(lastCharacter);

                if (isPathSeparatorCurrent && !isPathSeparatorLast)
                    parts.emplace_back(itBegin, itEnd);

                if (isPathSeparatorLast && !isPathSeparatorCurrent)
                    itBegin = itEnd;
            }

            ACE_TRY_ASSERT(!isPathSeparator(lastCharacter));
            parts.emplace_back(itBegin, end(path));

            ACE_TRY_ASSERT(parts.front().size() != 0);
            return parts;
        }));

        std::vector<std::filesystem::path>      finalFilePaths{};
        std::vector<Package::FileDirectoryPath> finalFileDirectoryPaths{};

        ACE_TRY_VOID(TransformExpectedVector(filePathsParts, [&]
        (const std::vector<std::string>& t_filePathParts) -> Expected<void>
        {
            ACE_TRY(firstFilePathPart, ([&]() -> Expected<std::string>
            {
                const auto& firstFilePathPart = t_filePathParts.front();

                if (!firstFilePathPart.starts_with("$"))
                    return firstFilePathPart;

                const std::string pathMacro{ begin(firstFilePathPart) + 1, end(firstFilePathPart) };

                const auto foundIt = t_pathMacroMap.find(pathMacro);
                ACE_TRY_ASSERT(foundIt != end(t_pathMacroMap));

                return foundIt->second;
            }()));

            struct LastFilePathPartData
            {
                std::optional<std::string> OptPath{};
                std::optional<std::string> OptExtension{};
                bool IsRecursive{};
            };

            ACE_TRY(lastFilePathPartData, ([&]() -> Expected<LastFilePathPartData>
            {
                std::string lastFilePathPart = t_filePathParts.back();
                TrimRight(lastFilePathPart);

                const size_t extensionDotPosition = lastFilePathPart.find_first_of('.');
                const auto optExtension = [&]() -> std::optional<std::string>
                {
                    const bool hasExtension = extensionDotPosition != std::string::npos;
                    if (!hasExtension)
                        return {};

                    return std::string
                    {
                        begin(lastFilePathPart) + extensionDotPosition,
                        end(lastFilePathPart)
                    };
                }();

                const std::string beforeExtension
                {
                    begin(lastFilePathPart),
                    begin(lastFilePathPart) + extensionDotPosition
                };

                const bool isFileDirectoryPath = beforeExtension[0] == '*';
                if (isFileDirectoryPath)
                {
                    ACE_TRY(isRecursive, [&]() -> Expected<bool>
                    {
                        if (beforeExtension.size() <= 1)
                        return false;

                    ACE_TRY_ASSERT(beforeExtension[1] == '*');
                    ACE_TRY_ASSERT(beforeExtension.size() == 2);
                    return true;
                    }());

                    return LastFilePathPartData
                    {
                        std::nullopt,
                        optExtension,
                        isRecursive
                    };
                }
                else
                {
                    return LastFilePathPartData
                    {
                        lastFilePathPart,
                        optExtension,
                        false
                    };
                }
            }()));

            std::string path = firstFilePathPart + '/';
            std::for_each(
                begin(t_filePathParts) + 1,
                end(t_filePathParts) - 1,
                [&](const std::string& t_part) { path += t_part + '/'; }
            );

            const bool isFilePath = lastFilePathPartData.OptPath.has_value();
            if (isFilePath)
            {
                path += lastFilePathPartData.OptPath.value();
                finalFilePaths.push_back(path);
            }
            else
            {
                finalFileDirectoryPaths.emplace_back(
                    path,
                    lastFilePathPartData.OptExtension,
                    lastFilePathPartData.IsRecursive
                );
            }

            return ExpectedVoid;
        }));

        return Package::FileDirectoryPaths
        {
            std::move(finalFilePaths),
            std::move(finalFileDirectoryPaths) 
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
            end(package[Property::FilePaths]),
            back_inserter(originalFilePaths),
            [](const nlohmann::json& t_filePath) { return t_filePath; }
        );
        ACE_TRY(finalFilePaths, TransformFilePaths(originalFilePaths, pathMacroMap));

        std::vector<std::string> originalDependencyFilePaths{};
        std::transform(
            begin(package[Property::DependencyFilePaths]),
            end(package[Property::DependencyFilePaths]),
            back_inserter(originalDependencyFilePaths),
            [](const nlohmann::json& t_filePath) { return t_filePath; }
        );
        ACE_TRY(finalDependencyFilePaths, TransformFilePaths(originalDependencyFilePaths, pathMacroMap));

        return Package
        {
            std::move(name),
            std::move(finalFilePaths),
            std::move(finalDependencyFilePaths)
        };
    }
}