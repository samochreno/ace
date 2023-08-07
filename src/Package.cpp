#include "Package.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <iterator>
#include <nlohmann/json.hpp>

#include "Diagnostic.hpp"
#include "Diagnostics/CompilationDiagnostics.hpp"
#include "Diagnostics/FileSystemDiagnostics.hpp"
#include "Diagnostics/JsonDiagnostics.hpp"
#include "String.hpp"
#include "Compilation.hpp"

namespace Ace
{
    namespace Property
    {
        static const std::string Name       = "name";
        static const std::string Value      = "value";
        static const std::string PathMacros = "path_macros";
        static const std::string SrcFiles   = "src_files";
        static const std::string DepFiles   = "dep_files";
    }

    enum class Recursiveness
    {
        Recursive,
        NonRecursive,
    };

    struct FilteredDirectory
    {
        FilteredDirectory(
            const std::filesystem::path& path, 
            const std::optional<std::string>& optExtensionFilter, 
            const Recursiveness recursiveness
        ) : Path{ path }, 
            OptExtensionFilter{ optExtensionFilter }, 
            Recursiveness{ recursiveness }
        {
        }

        std::filesystem::path Path{};
        std::optional<std::string> OptExtensionFilter{};
        Recursiveness Recursiveness{};
    };

    static auto DoesDirectoryEntryMatchFilter(
        const std::filesystem::directory_entry& directoryEntry,
        std::optional<std::string> optExtensionFilter
    ) -> bool
    {
        if (!directoryEntry.is_regular_file())
        {
            return false;
        }

        if (!optExtensionFilter.has_value())
        {
            return true;
        }

        const auto& path = directoryEntry.path();
        if (!path.has_extension())
        {
            return false;
        }

        if (path.extension() != optExtensionFilter.value())
        {
            return false;
        }

        return true;
    }

    static auto CollectFilteredDirectoryFilePaths(
        const FilteredDirectory& directory
    ) -> Diagnosed<std::vector<std::filesystem::path>>
    {
        DiagnosticBag diagnostics{};

        if (!std::filesystem::exists(directory.Path))
        {
            return Diagnosed
            {
                std::vector<std::filesystem::path>{},
                diagnostics
            };
        }

        std::vector<std::filesystem::path> filePaths{};
        try
        {
            if (directory.Recursiveness == Recursiveness::Recursive)
            {
                for (
                    const auto& directoryEntry :
                    std::filesystem::recursive_directory_iterator(directory.Path)
                    )
                {
                    const bool doesMatch = DoesDirectoryEntryMatchFilter(
                        directoryEntry,
                        directory.OptExtensionFilter
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
                    std::filesystem::directory_iterator(directory.Path)
                    )
                {
                    const bool doesMatch = DoesDirectoryEntryMatchFilter(
                        directoryEntry,
                        directory.OptExtensionFilter
                    );
                    if (!doesMatch)
                    {
                        continue;
                    }

                    filePaths.push_back(directoryEntry.path());
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            diagnostics.Add(CreateFileSystemError(directory.Path, e));
        }

        return Diagnosed{ filePaths, diagnostics };
    }

    struct ExpandedLastFilePathPartData
    {
        std::optional<std::string> OptPath{};
        std::optional<std::string> OptExtension{};
        Recursiveness Recursiveness{};
    };

    static auto ExpandLastFilePathPart(
        const FileBuffer* const fileBuffer,
        std::string part
    ) -> Diagnosed<ExpandedLastFilePathPartData>
    {
        DiagnosticBag diagnostics{};

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
            return Diagnosed
            {
                ExpandedLastFilePathPartData
                {
                    part,
                    optExtension,
                    Recursiveness::NonRecursive,
                },
                diagnostics,
            };
        }

        const auto recursiveness = [&]() -> Recursiveness
        {
            if (beforeExtension.size() <= 1)
            {
                return Recursiveness::NonRecursive;
            }

            auto recursiveness = Recursiveness::NonRecursive;
            auto it = begin(beforeExtension) + 1;
            if (*it == '*')
            {
                recursiveness = Recursiveness::Recursive;
                ++it;
            }

            if (it != end(beforeExtension))
            {
                diagnostics.Add(CreateTrailingPackagePathCharactersBeforeExtensionError(
                    fileBuffer,
                    std::string_view{ it, end(beforeExtension) }
                ));
            }

            return recursiveness;
        }();

        return Diagnosed
        {
            ExpandedLastFilePathPartData
            {
                std::nullopt,
                optExtension,
                recursiveness,
            },
            diagnostics,
        };
    }

    class FilePathOrFilteredDirectory
    {
    public:
        FilePathOrFilteredDirectory() = default;
        FilePathOrFilteredDirectory(
            const std::filesystem::path& filePath
        ) : m_OptFilePath{ filePath }
        {
        }
        FilePathOrFilteredDirectory(
            const FilteredDirectory& directoryPath
        ) : m_OptFilteredDirectory{ directoryPath }
        {
        }
        ~FilePathOrFilteredDirectory() = default;

        auto IsFilePath() const -> bool
        {
            return m_OptFilePath.has_value();
        }
        auto IsDirectoryPath() const -> bool
        {
            return m_OptFilteredDirectory.has_value();
        }

        auto GetFilePath() const -> const std::filesystem::path&
        {
            return m_OptFilePath.value();
        }
        auto GetFilteredDirectory() const -> const FilteredDirectory&
        {
            return m_OptFilteredDirectory.value();
        }

    private:
        std::optional<std::filesystem::path> m_OptFilePath{};
        std::optional<FilteredDirectory> m_OptFilteredDirectory{};
    };

    static auto ExpandFirstFilePathPart(
        const FileBuffer* const fileBuffer,
        const std::string& part,
        const std::unordered_map<std::string, std::string>& pathMacroMap
    ) -> Diagnosed<std::string>
    {
        DiagnosticBag diagnostics{};

        const bool isMacro = part.starts_with('$');
        if (!isMacro)
        {
            return Diagnosed{ part, diagnostics };
        }

        const std::string pathMacro = part.substr(1);

        const auto macroValueIt = pathMacroMap.find(pathMacro);
        if (macroValueIt == end(pathMacroMap))
        {
            diagnostics.Add(CreateUndefinedRefToPackagePathMacroError(
                fileBuffer,
                pathMacro
            ));
            return Diagnosed{ std::string{}, diagnostics };
        }
        
        return Diagnosed{ macroValueIt->second, diagnostics };
    }

    static auto ExpandFilePathParts(
        const FileBuffer* const fileBuffer,
        const std::vector<std::string>& filePathParts,
        const std::unordered_map<std::string, std::string>& pathMacroMap
    ) -> Diagnosed<FilePathOrFilteredDirectory>
    {
        DiagnosticBag diagnostics{};

        const auto firstFilePathPart = diagnostics.Collect(ExpandFirstFilePathPart(
            fileBuffer,
            filePathParts.front(),
            pathMacroMap
        ));

        const auto lastFilePathPartData = diagnostics.Collect(ExpandLastFilePathPart(
            fileBuffer,
            filePathParts.back()
        ));

        std::string path = firstFilePathPart + '/';
        std::for_each(
            begin(filePathParts) + 1,
            end  (filePathParts) - 1,
            [&](const std::string& part) { path += part + '/'; }
        );

        const bool isFilePath = lastFilePathPartData.OptPath.has_value();
        if (isFilePath)
        {
            path += lastFilePathPartData.OptPath.value();
            return Diagnosed
            {
                FilePathOrFilteredDirectory{ path },
                diagnostics
            };
        }

        return Diagnosed
        {
            FilePathOrFilteredDirectory
            {
                FilteredDirectory
                {
                    path,
                    lastFilePathPartData.OptExtension,
                    lastFilePathPartData.Recursiveness,
                }
            },
            diagnostics,
        };
    }

    static auto SplitFilePath(
        const std::string& filePath
    ) -> Expected<std::vector<std::string>>
    {
        DiagnosticBag diagnostics{};

        const auto isPathSeparator = [](const char& character) -> bool
        {
            return (character == '/') || (character == '\\');
        };

        std::vector<std::string> parts{};

        auto itBegin = begin(filePath);
        auto itEnd   = begin(filePath);

        char character{};
        char lastCharacter{};

        for (; itEnd != end(filePath); [&]() { lastCharacter = character; ++itEnd; }())
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

        if (!isPathSeparator(character))
        {
            parts.emplace_back(itBegin, end(filePath));
        }
        else
        {
            return diagnostics.Add(CreateFilePathEndsWithSeparatorError(
                filePath
            ));
        }

        return Expected{ parts, diagnostics };
    }

    static auto TransformFilePaths(
        const FileBuffer* const fileBuffer,
        const std::filesystem::path& packageFilePath,
        const std::vector<std::string>& filePaths,
        const std::unordered_map<std::string, std::string>& pathMacroMap
    ) -> Diagnosed<std::vector<std::filesystem::path>>
    {
        DiagnosticBag diagnostics{};

        std::vector<std::vector<std::string>> filePathsParts{};
        std::for_each(begin(filePaths), end(filePaths),
        [&](const std::string& filePath)
        {
            const auto optFilePathParts = diagnostics.Collect(SplitFilePath(
                filePath
            ));
            if (!optFilePathParts.has_value())
            {
                return;
            }

            filePathsParts.push_back(optFilePathParts.value());
        });

        const auto packageDirectoryPath = packageFilePath.parent_path();

        std::vector<std::filesystem::path> finalFilePaths{};
        std::for_each(begin(filePathsParts), end(filePathsParts),
        [&](const std::vector<std::string>& filePathParts)
        {
            const auto filePathOrFilteredDirectory = diagnostics.Collect(ExpandFilePathParts(
                fileBuffer,
                filePathParts,
                pathMacroMap
            ));

            if (filePathOrFilteredDirectory.IsFilePath())
            {
                const auto expandedFilePath =
                    filePathOrFilteredDirectory.GetFilePath();

                const auto filePath = [&]() -> std::filesystem::path
                {
                    if (expandedFilePath.is_absolute())
                    {
                        return expandedFilePath;
                    }

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
                    diagnostics.Collect(CollectFilteredDirectoryFilePaths(directory));

                finalFilePaths.insert(
                    end(finalFilePaths),
                    begin(directoryFilePaths),
                    end  (directoryFilePaths)
                );
            }
        });

        return Diagnosed{ finalFilePaths, diagnostics };
    }

    static auto ReadFilePath(
        std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
        const FileBuffer* const fileBuffer,
        const std::filesystem::path& filePath
    ) -> Expected<const FileBuffer*>
    {
        DiagnosticBag diagnostics{};

        const auto optFileBuffer = diagnostics.Collect(FileBuffer::Read(
            fileBuffer->GetCompilation(),
            filePath
        ));
        if (!optFileBuffer.has_value())
        {
            return diagnostics;
        }

        srcBuffers->push_back(optFileBuffer.value());

        return Expected{ optFileBuffer.value().get(), diagnostics };
    }

    static auto CreateDefaultValue(
        const nlohmann::json::value_t type
    ) -> nlohmann::json
    {
        switch (type)
        {
            case nlohmann::json::value_t::null:            return nullptr;
            case nlohmann::json::value_t::object:          return nlohmann::json::object();
            case nlohmann::json::value_t::array:           return nlohmann::json::array();
            case nlohmann::json::value_t::string:          return std::string{};
            case nlohmann::json::value_t::boolean:         return false;
            case nlohmann::json::value_t::number_integer:  return static_cast< int32_t>(0);
            case nlohmann::json::value_t::number_unsigned: return static_cast<uint32_t>(0);
            case nlohmann::json::value_t::number_float:    return 0.0f;
            case nlohmann::json::value_t::binary:          ACE_UNREACHABLE();
            case nlohmann::json::value_t::discarded:       ACE_UNREACHABLE();
        }
    }

    static auto GetOrCreateProperty(
        const FileBuffer* const fileBuffer,
        const nlohmann::json& json,
        const std::string& namePrefix,
        const std::string& name,
        const nlohmann::json::value_t type
    ) -> Diagnosed<nlohmann::json>
    {
        DiagnosticBag diagnostics{};

        const auto prefixedName = namePrefix.empty() ?
            name :
            (namePrefix + "." + name);

        if (!json.contains(name))
        {
            diagnostics.Add(CreateMissingPackagePropertyError(
                fileBuffer,
                prefixedName
            ));
            return Diagnosed{ CreateDefaultValue(type), diagnostics };
        }

        if (json[name].type() != type)
        {
            diagnostics.Add(CreateUnexpectedPackagePropertyTypeError(
                fileBuffer,
                prefixedName,
                json[name].type(),
                type
            ));
            return Diagnosed{ CreateDefaultValue(type), diagnostics };
        }

        return Diagnosed{ json[name], diagnostics };
    }

    static auto GetOrCreateElement(
        const FileBuffer* const fileBuffer,
        const nlohmann::json& json,
        const std::string& namePrefix,
        const size_t index,
        const nlohmann::json::value_t type
    ) -> Diagnosed<nlohmann::json>
    {
        DiagnosticBag diagnostics{};

        ACE_ASSERT(index < json.size());

        if (json.at(index).type() != type)
        {
            diagnostics.Add(CreateUnexpectedPackagePropertyTypeError(
                fileBuffer,
                namePrefix + "[" + std::to_string(index) + "]",
                json.at(index).type(),
                type
            ));
            return Diagnosed{ CreateDefaultValue(type), diagnostics };
        }

        return Diagnosed{ json.at(index), diagnostics };
    }

    static auto DiagnoseUnexpectedProperties(
        const FileBuffer* const fileBuffer,
        const nlohmann::json& json,
        const std::string& namePrefix,
        std::vector<std::string> expectedNames
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        for (const auto& nameValuePair : json.items())
        {
            const auto& name  = nameValuePair.key();
            const auto& value = nameValuePair.value();

            const auto matchingNameIt = std::find_if(
                begin(expectedNames),
                end  (expectedNames),
                [&](const std::string& expectedName)
                {
                    return name == expectedName;
                }
            );
            if (matchingNameIt != end(expectedNames))
            {
                expectedNames.erase(matchingNameIt);
            }
            else
            {
                diagnostics.Add(CreateUnexpectedPackagePropertyWarning(
                    fileBuffer,
                    namePrefix.empty() ? name : (namePrefix + "." + name)
                ));
            }
        }

        return Diagnosed<void>{ diagnostics };
    }

    static auto ParsePathMacroMap(
        const FileBuffer* const fileBuffer,
        const nlohmann::json& package
    ) -> Diagnosed<std::unordered_map<std::string, std::string>>
    {
        DiagnosticBag diagnostics{};

        const auto pathMacros = diagnostics.Collect(GetOrCreateProperty(
            fileBuffer,
            package,
            {},
            Property::PathMacros,
            nlohmann::json::value_t::array
        ));

        std::unordered_map<std::string, std::string> pathMacroMap{};
        for (size_t i = 0; i < pathMacros.size(); i++)
        {
            const auto pathMacro = diagnostics.Collect(GetOrCreateElement(
                fileBuffer,
                pathMacros,
                Property::PathMacros,
                i,
                nlohmann::json::value_t::object
            ));

            const auto pathMacroPropertyNamePrefix =
                Property::PathMacros + "[" + std::to_string(i) + "]";

            diagnostics.Collect(DiagnoseUnexpectedProperties(
                fileBuffer,
                pathMacro,
                pathMacroPropertyNamePrefix,
                {
                    Property::Name,
                    Property::Value,
                }
            ));

            std::string pathMacroName = diagnostics.Collect(GetOrCreateProperty(
                fileBuffer,
                pathMacro,
                pathMacroPropertyNamePrefix,
                Property::Name,
                nlohmann::json::value_t::string
            ));
            Trim(pathMacroName);

            std::string pathMacroValue = diagnostics.Collect(GetOrCreateProperty(
                fileBuffer,
                pathMacro,
                pathMacroPropertyNamePrefix,
                Property::Value,
                nlohmann::json::value_t::string
            ));
            Trim(pathMacroValue);

            pathMacroMap[pathMacroName] = std::move(pathMacroValue);
        }

        return Diagnosed{ pathMacroMap, diagnostics };
    }

    static auto ParseFiles(
        const FileBuffer* const fileBuffer,
        const nlohmann::json& package,
        const std::string& filesPropertyName
    ) -> Diagnosed<std::vector<std::string>>
    {
        DiagnosticBag diagnostics{};

        const auto filesArray = diagnostics.Collect(GetOrCreateProperty(
            fileBuffer,
            package,
            {},
            filesPropertyName,
            nlohmann::json::value_t::array
        ));

        std::vector<std::string> files{};
        for (size_t i = 0; i < filesArray.size(); i++)
        {
            const auto file = diagnostics.Collect(GetOrCreateElement(
                fileBuffer,
                filesArray,
                filesPropertyName,
                i,
                nlohmann::json::value_t::string
            ));

            files.push_back(file);
        }

        return Diagnosed{ files, diagnostics };
    }

    static auto ReadFilePaths(
        std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
        const FileBuffer* const fileBuffer,
        const std::vector<std::filesystem::path>& filePaths
    ) -> Diagnosed<std::vector<const FileBuffer*>>
    {
        DiagnosticBag diagnostics{};

        std::vector<const FileBuffer*> fileBuffers{};
        std::for_each(begin(filePaths), end(filePaths),
        [&](const std::filesystem::path& filePath)
        {
            const auto optFileBuffer = diagnostics.Collect(ReadFilePath(
                srcBuffers,
                fileBuffer,
                filePath
            ));
            if (!optFileBuffer.has_value())
            {
                return;
            }

            fileBuffers.push_back(optFileBuffer.value());
        });

        return Diagnosed{ fileBuffers, diagnostics };
    }

    static auto ParsePackage(
        std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
        const FileBuffer* const fileBuffer
    ) -> Diagnosed<Package>
    {
        DiagnosticBag diagnostics{};

        const auto package = nlohmann::json::parse(fileBuffer->GetBuffer());

        diagnostics.Collect(DiagnoseUnexpectedProperties(
            fileBuffer,
            package,
            {},
            {
                Property::Name,
                Property::PathMacros,
                Property::SrcFiles,
                Property::DepFiles,
            }
        ));

        auto name = diagnostics.Collect(GetOrCreateProperty(
            fileBuffer,
            package,
            {},
            Property::Name,
            nlohmann::json::value_t::string
        ));

        const auto pathMacroMap = diagnostics.Collect(ParsePathMacroMap(
            fileBuffer,
            package
        ));

        const auto srcFiles = diagnostics.Collect(ParseFiles(
            fileBuffer,
            package,
            Property::SrcFiles
        ));

        const auto depFiles = diagnostics.Collect(ParseFiles(
            fileBuffer,
            package,
            Property::DepFiles
        ));

        const auto srcFilePaths = diagnostics.Collect(TransformFilePaths(
            fileBuffer,
            fileBuffer->GetPath(),
            srcFiles,
            pathMacroMap
        ));

        const auto depFilePaths = diagnostics.Collect(TransformFilePaths(
            fileBuffer,
            fileBuffer->GetPath(),
            depFiles,
            pathMacroMap
        ));

        const auto srcFileBuffers = diagnostics.Collect(ReadFilePaths(
            srcBuffers,
            fileBuffer,
            srcFilePaths
        ));

        return Diagnosed
        {
            Package
            {
                std::move(name),
                std::move(srcFileBuffers),
                std::move(depFilePaths),
            },
            diagnostics,
        };
    }

    auto Package::Parse(
        std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
        const FileBuffer* const fileBuffer
    ) -> Expected<Package>
    {
        DiagnosticBag diagnostics{};

        try
        {
            const auto package = diagnostics.Collect(ParsePackage(
                srcBuffers,
                fileBuffer
            ));

            return Expected{ package, diagnostics };
        }
        catch (const nlohmann::json::exception& exception)
        {
            return diagnostics.Add(CreateJsonError(
                fileBuffer,
                exception
            ));
        }
    }
}
