#include "Package.hpp"

#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <iterator>
#include <nlohmann/json.hpp>

#include "Diagnostics.hpp"
#include "Utility.hpp"
#include "Compilation.hpp"

namespace Ace
{
    namespace Property
    {
        static const std::string Name                = "name";
        static const std::string Value               = "value";
        static const std::string PathMacros          = "path_macros";
        static const std::string SourceFiles         = "source_files";
        static const std::string DependencyFiles     = "dependency_files";
    }

    enum class Recursiveness
    {
        Recursive,
        NonRecursive,
    };

    struct FilteredDirectory
    {
        FilteredDirectory(
            const std::filesystem::path& t_path, 
            const std::optional<std::string>& t_optExtensionFilter, 
            const Recursiveness t_recursiveness
        ) : Path{ t_path }, 
            OptExtensionFilter{ t_optExtensionFilter }, 
            Recursiveness{ t_recursiveness }
        {
        }

        std::filesystem::path Path{};
        std::optional<std::string> OptExtensionFilter{};
        Recursiveness Recursiveness{};
    };

    static auto DoesDirectoryEntryMatchFilter(
        const std::filesystem::directory_entry& t_directoryEntry,
        std::optional<std::string> t_optExtensionFilter
    ) -> bool
    {
        if (!t_directoryEntry.is_regular_file())
        {
            return false;
        }

        if (!t_optExtensionFilter.has_value())
        {
            return true;
        }

        const auto& path = t_directoryEntry.path();
        if (!path.has_extension())
        {
            return false;
        }

        if (path.extension() != t_optExtensionFilter.value())
        {
            return false;
        }

        return true;
    }

    static auto CollectFilteredDirectoryFilePaths(
        const FilteredDirectory& t_directory
    ) -> std::vector<std::filesystem::path>
    {
        if (!std::filesystem::exists(t_directory.Path))
        {
            return {};
        }

        std::vector<std::filesystem::path> filePaths{};

        if (t_directory.Recursiveness == Recursiveness::Recursive)
        {
            for (
                const auto& directoryEntry :
                std::filesystem::recursive_directory_iterator(t_directory.Path)
                )
            {
                const bool doesMatch = DoesDirectoryEntryMatchFilter(
                    directoryEntry,
                    t_directory.OptExtensionFilter
                );
                if (!doesMatch)
                {
                    continue;
                }

                filePaths.push_back(directoryEntry.path());
            }
        }
        else
        {
            for (
                const auto& directoryEntry :
                std::filesystem::directory_iterator(t_directory.Path)
                )
            {
                const bool doesMatch = DoesDirectoryEntryMatchFilter(
                    directoryEntry,
                    t_directory.OptExtensionFilter
                );
                if (!doesMatch)
                {
                    continue;
                }

                filePaths.push_back(directoryEntry.path());
            }
        }

        return filePaths;
    }

    struct ExpandedLastFilePathPart
    {
        std::optional<std::string> OptPath{};
        std::optional<std::string> OptExtension{};
        Recursiveness Recursiveness{};
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
            {
                return std::nullopt;
            }

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
        if (!isFileDirectoryPath)
        {
            return ExpandedLastFilePathPart
            {
                part,
                optExtension,
                Recursiveness::NonRecursive,
            };
        }

        ACE_TRY(recursiveness, [&]() -> Expected<Recursiveness>
        {
            if (beforeExtension.size() <= 1)
            {
                return Recursiveness::NonRecursive;
            }

            ACE_TRY_ASSERT(beforeExtension[1] == '*');
            ACE_TRY_ASSERT(beforeExtension.size() == 2);
            return Recursiveness::Recursive;
        }());

        return ExpandedLastFilePathPart
        {
            std::nullopt,
            optExtension,
            recursiveness,
        };
    }

    class FilePathOrFilteredDirectory
    {
    public:
        FilePathOrFilteredDirectory() = default;
        FilePathOrFilteredDirectory(
            const std::filesystem::path& t_filePath
        ) : m_OptFilePath{ t_filePath }
        {
        }
        FilePathOrFilteredDirectory(const FilteredDirectory& t_directoryPath)
            : m_OptFilteredDirectory{ t_directoryPath }
        {
        }
        ~FilePathOrFilteredDirectory() = default;

        auto IsFilePath()      const -> bool { return m_OptFilePath.has_value(); }
        auto IsDirectoryPath() const -> bool { return m_OptFilteredDirectory.has_value(); }

        auto GetFilePath()          -> const std::filesystem::path& { return m_OptFilePath.value(); }
        auto GetFilteredDirectory() -> FilteredDirectory&           { return m_OptFilteredDirectory.value(); }

    private:
        std::optional<std::filesystem::path> m_OptFilePath{};
        std::optional<FilteredDirectory>     m_OptFilteredDirectory{};
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

        const auto macroValueIt = t_pathMacroMap.find(pathMacro);
        ACE_TRY_ASSERT(macroValueIt != end(t_pathMacroMap));

        return macroValueIt->second;
    }

    static auto ExpandFilePathParts(
        const std::vector<std::string>& t_filePathParts,
        const std::unordered_map<std::string, std::string>& t_pathMacroMap
    ) -> Expected<FilePathOrFilteredDirectory>
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
            return FilePathOrFilteredDirectory{ path };
        }

        return FilePathOrFilteredDirectory
        {
            FilteredDirectory
            {
                path,
                lastFilePathPartData.OptExtension,
                lastFilePathPartData.Recursiveness
            }
        };
    }

    static auto SplitFilePath(
        const std::string& t_filePath
    ) -> Expected<std::vector<std::string>>
    {
        auto isPathSeparator = [](const char& t_character) -> bool
        {
            return (t_character == '/') || (t_character == '\\');
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

        ACE_TRY_ASSERT(!parts.front().empty());
        return parts;
    }

    static auto TransformFilePaths(
        const std::filesystem::path& t_packageFilePath,
        const std::vector<std::string>& t_filePaths,
        const std::unordered_map<std::string, std::string>& t_pathMacroMap
    ) -> Expected<std::vector<std::filesystem::path>>
    {
        ACE_TRY(filePathsParts, TransformExpectedVector(t_filePaths,
        [&](const std::string& t_filePath) -> Expected<std::vector<std::string>>
        {
            return SplitFilePath(t_filePath);
        }));

        const auto packageDirectoryPath = t_packageFilePath.parent_path();

        std::vector<std::filesystem::path> finalFilePaths{};
        ACE_TRY_VOID(TransformExpectedVector(filePathsParts,
        [&](const std::vector<std::string>& t_filePathParts) -> Expected<void>
        {
            ACE_TRY(filePathOrFilteredDirectory, ExpandFilePathParts(
                t_filePathParts,
                t_pathMacroMap
            ));

            if (filePathOrFilteredDirectory.IsFilePath())
            {
                const auto expandedFilePath =
                    filePathOrFilteredDirectory.GetFilePath();

                const auto filePath = [&]() -> std::filesystem::path
                {
                    if (expandedFilePath.is_absolute())
                        return expandedFilePath;

                    return packageDirectoryPath / expandedFilePath;
                }();

                finalFilePaths.push_back(filePath);
            }

            if (filePathOrFilteredDirectory.IsDirectoryPath())
            {
                auto directory =
                    filePathOrFilteredDirectory.GetFilteredDirectory();

                directory.Path = directory.Path.is_absolute() ? 
                    directory.Path : 
                    (packageDirectoryPath / directory.Path);

                const auto directoryFilePaths =
                    CollectFilteredDirectoryFilePaths(directory);

                finalFilePaths.insert(
                    end(finalFilePaths),
                    begin(directoryFilePaths),
                    end  (directoryFilePaths)
                );
            }

            return Void;
        }));

        return finalFilePaths;
    }

    static auto ReadFilePath(
        std::vector<std::shared_ptr<const ISourceBuffer>>* const t_sourceBuffers,
        const FileBuffer* const t_fileBuffer,
        const std::filesystem::path& t_filePath
    ) -> Expected<const FileBuffer*>
    {
        DiagnosticBag diagnosticBag{};

        const auto expFileBuffer = FileBuffer::Read(
            t_fileBuffer->GetCompilation(),
            t_filePath
        );
        diagnosticBag.Add(expFileBuffer);
        if (!expFileBuffer)
        {
            return diagnosticBag;
        }

        t_sourceBuffers->push_back(expFileBuffer.Unwrap());

        return
        {
            expFileBuffer.Unwrap().get(),
            diagnosticBag,
        };
    }

    static auto ParsePackage(
        std::vector<std::shared_ptr<const ISourceBuffer>>* const t_sourceBuffers,
        const FileBuffer* const t_fileBuffer
    ) -> Expected<Package>
    {
        DiagnosticBag diagnosticBag{};

        const auto package = nlohmann::json::parse(t_fileBuffer->GetBuffer());

        ACE_TRY_ASSERT(package.contains(Property::Name));
        ACE_TRY_ASSERT(package.contains(Property::PathMacros));
        ACE_TRY_ASSERT(package.contains(Property::SourceFiles));
        ACE_TRY_ASSERT(package.contains(Property::DependencyFiles));
        ACE_TRY_ASSERT(package.size() == 4);

        std::string name = package[Property::Name];
        
        std::unordered_map<std::string, std::string> pathMacroMap{};
        for (const auto& pathMacro : package[Property::PathMacros])
        {
            ACE_TRY_ASSERT(pathMacro.contains(Property::Name));
            ACE_TRY_ASSERT(pathMacro.contains(Property::Value));
            ACE_TRY_ASSERT(pathMacro.size() == 2);

            const std::string value = pathMacro[Property::Name];
            ACE_TRY_ASSERT(!value.empty());
        
            std::string path = pathMacro[Property::Value];
            pathMacroMap[pathMacro[Property::Name]] = std::move(Trim(path));
        }

        std::vector<std::string> originalSourceFilePaths{};
        std::transform(
            begin(package[Property::SourceFiles]),
            end  (package[Property::SourceFiles]),
            back_inserter(originalSourceFilePaths),
            [](const nlohmann::json& t_filePath) { return t_filePath; }
        );
        ACE_TRY(finalSourceFilePaths, TransformFilePaths(
            t_fileBuffer->GetPath(),
            originalSourceFilePaths,
            pathMacroMap
        ));
        ACE_TRY(sourceFileBuffers, TransformExpectedVector(finalSourceFilePaths,
        [&](const std::filesystem::path& t_filePath) -> Expected<const FileBuffer*>
        {
            return ReadFilePath(
                t_sourceBuffers,
                t_fileBuffer,
                t_filePath
            );
        }));

        std::vector<std::string> originalDependencyFilePaths{};
        std::transform(
            begin(package[Property::DependencyFiles]),
            end  (package[Property::DependencyFiles]),
            back_inserter(originalDependencyFilePaths),
            [](const nlohmann::json& t_filePath) { return t_filePath; }
        );
        ACE_TRY(finalDependencyFilePaths, TransformFilePaths(
            t_fileBuffer->GetPath(),
            originalDependencyFilePaths,
            pathMacroMap
        ));

        return
        {
            Package
            {
                std::move(name),
                std::move(sourceFileBuffers),
                std::move(finalDependencyFilePaths),
            },
            diagnosticBag,
        };
    }

    auto Package::Parse(
        std::vector<std::shared_ptr<const ISourceBuffer>>* const t_sourceBuffers,
        const FileBuffer* const t_fileBuffer
    ) -> Expected<Package>
    {
        DiagnosticBag diagnosticBag{};

        try
        {
            auto expPackage = ParsePackage(t_sourceBuffers, t_fileBuffer);
            diagnosticBag.Add(expPackage);
            if (!expPackage)
            {
                return diagnosticBag;
            }

            return
            {
                std::move(expPackage.Unwrap()),
                diagnosticBag,
            };
        }
        catch (const nlohmann::json::exception& exception)
        {
            return diagnosticBag.Add<JsonError>(t_fileBuffer, exception);
        }
    }
}
