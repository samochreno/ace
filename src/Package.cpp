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
        static const std::string Name            = "name";
        static const std::string Value           = "value";
        static const std::string PathMacros      = "path_macros";
        static const std::string SourceFiles     = "source_files";
        static const std::string DependencyFiles = "dependency_files";
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
        DiagnosticBag diagnosticBag{};

        if (!std::filesystem::exists(directory.Path))
        {
            return { {}, diagnosticBag };
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
            diagnosticBag.Add(CreateFileSystemError(directory.Path, e));
        }

        return { filePaths, diagnosticBag };
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
        DiagnosticBag diagnosticBag{};

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
            return
            {
                ExpandedLastFilePathPartData
                {
                    part,
                    optExtension,
                    Recursiveness::NonRecursive,
                },
                diagnosticBag,
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
                diagnosticBag.Add(CreateTrailingPackagePathCharactersBeforeExtensionError(
                    fileBuffer,
                    std::string_view{ it, end(beforeExtension) }
                ));
            }

            return recursiveness;
        }();

        return
        {
            ExpandedLastFilePathPartData
            {
                std::nullopt,
                optExtension,
                recursiveness,
            },
            diagnosticBag,
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
        DiagnosticBag diagnosticBag{};

        const bool isMacro = part.starts_with('$');
        if (!isMacro)
        {
            return { part, diagnosticBag };
        }

        const std::string pathMacro = part.substr(1);

        const auto macroValueIt = pathMacroMap.find(pathMacro);
        if (macroValueIt == end(pathMacroMap))
        {
            diagnosticBag.Add(CreateUndefinedReferenceToPackagePathMacroError(
                fileBuffer,
                pathMacro
            ));

            return { {}, diagnosticBag };
        }
        
        return { macroValueIt->second, diagnosticBag };
    }

    static auto ExpandFilePathParts(
        const FileBuffer* const fileBuffer,
        const std::vector<std::string>& filePathParts,
        const std::unordered_map<std::string, std::string>& pathMacroMap
    ) -> Diagnosed<FilePathOrFilteredDirectory>
    {
        DiagnosticBag diagnosticBag{};

        const auto dgnFirstFilePathPart = ExpandFirstFilePathPart(
            fileBuffer,
            filePathParts.front(),
            pathMacroMap
        );
        diagnosticBag.Add(dgnFirstFilePathPart);

        const auto dgnLastFilePathPartData = ExpandLastFilePathPart(
            fileBuffer,
            filePathParts.back()
        );
        diagnosticBag.Add(dgnLastFilePathPartData);

        std::string path = dgnFirstFilePathPart.Unwrap() + '/';
        std::for_each(
            begin(filePathParts) + 1,
            end  (filePathParts) - 1,
            [&](const std::string& part) { path += part + '/'; }
        );

        const bool isFilePath = dgnLastFilePathPartData.Unwrap().OptPath.has_value();
        if (isFilePath)
        {
            path += dgnLastFilePathPartData.Unwrap().OptPath.value();
            return { FilePathOrFilteredDirectory{ path }, diagnosticBag };
        }

        return
        {
            FilePathOrFilteredDirectory
            {
                FilteredDirectory
                {
                    path,
                    dgnLastFilePathPartData.Unwrap().OptExtension,
                    dgnLastFilePathPartData.Unwrap().Recursiveness,
                }
            },
            diagnosticBag,
        };
    }

    static auto SplitFilePath(
        const std::string& filePath
    ) -> Expected<std::vector<std::string>>
    {
        DiagnosticBag diagnosticBag{};

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
            return diagnosticBag.Add(CreateFilePathEndsWithSeparatorError(
                filePath
            ));
        }

        return { parts, diagnosticBag };
    }

    static auto TransformFilePaths(
        const FileBuffer* const fileBuffer,
        const std::filesystem::path& packageFilePath,
        const std::vector<std::string>& filePaths,
        const std::unordered_map<std::string, std::string>& pathMacroMap
    ) -> Diagnosed<std::vector<std::filesystem::path>>
    {
        DiagnosticBag diagnosticBag{};

        std::vector<std::vector<std::string>> filePathsParts{};
        std::for_each(begin(filePaths), end(filePaths),
        [&](const std::string& filePath)
        {
            const auto expFilePathParts = SplitFilePath(filePath);
            diagnosticBag.Add(expFilePathParts);
            if (!expFilePathParts)
            {
                return;
            }

            filePathsParts.push_back(expFilePathParts.Unwrap());
        });

        const auto packageDirectoryPath = packageFilePath.parent_path();

        std::vector<std::filesystem::path> finalFilePaths{};
        std::for_each(begin(filePathsParts), end(filePathsParts),
        [&](const std::vector<std::string>& filePathParts)
        {
            const auto dgnFilePathOrFilteredDirectory = ExpandFilePathParts(
                fileBuffer,
                filePathParts,
                pathMacroMap
            );
            diagnosticBag.Add(dgnFilePathOrFilteredDirectory);

            if (dgnFilePathOrFilteredDirectory.Unwrap().IsFilePath())
            {
                const auto expandedFilePath =
                    dgnFilePathOrFilteredDirectory.Unwrap().GetFilePath();

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

            if (dgnFilePathOrFilteredDirectory.Unwrap().IsDirectoryPath())
            {
                auto directory =
                    dgnFilePathOrFilteredDirectory.Unwrap().GetFilteredDirectory();

                directory.Path = directory.Path.is_absolute() ? 
                    directory.Path : 
                    (packageDirectoryPath / directory.Path);

                const auto dgnDirectoryFilePaths =
                    CollectFilteredDirectoryFilePaths(directory);
                diagnosticBag.Add(dgnDirectoryFilePaths);

                finalFilePaths.insert(
                    end(finalFilePaths),
                    begin(dgnDirectoryFilePaths.Unwrap()),
                    end  (dgnDirectoryFilePaths.Unwrap())
                );
            }
        });

        return { finalFilePaths, diagnosticBag };
    }

    static auto ReadFilePath(
        std::vector<std::shared_ptr<const ISourceBuffer>>* const sourceBuffers,
        const FileBuffer* const fileBuffer,
        const std::filesystem::path& filePath
    ) -> Expected<const FileBuffer*>
    {
        DiagnosticBag diagnosticBag{};

        const auto expFileBuffer = FileBuffer::Read(
            fileBuffer->GetCompilation(),
            filePath
        );
        diagnosticBag.Add(expFileBuffer);
        if (!expFileBuffer)
        {
            return diagnosticBag;
        }

        sourceBuffers->push_back(expFileBuffer.Unwrap());

        return { expFileBuffer.Unwrap().get(), diagnosticBag };
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
        DiagnosticBag diagnosticBag{};

        const auto prefixedName = namePrefix.empty() ?
            name :
            (namePrefix + "." + name);

        if (!json.contains(name))
        {
            diagnosticBag.Add(CreateMissingPackagePropertyError(
                fileBuffer,
                prefixedName
            ));
            return { CreateDefaultValue(type), diagnosticBag };
        }

        if (json[name].type() != type)
        {
            diagnosticBag.Add(CreateUnexpectedPackagePropertyTypeError(
                fileBuffer,
                prefixedName,
                json[name].type(),
                type
            ));
            return { CreateDefaultValue(type), diagnosticBag };
        }

        return { json[name], diagnosticBag };
    }

    static auto GetOrCreateElement(
        const FileBuffer* const fileBuffer,
        const nlohmann::json& json,
        const std::string& namePrefix,
        const size_t index,
        const nlohmann::json::value_t type
    ) -> Diagnosed<nlohmann::json>
    {
        DiagnosticBag diagnosticBag{};

        ACE_ASSERT(index < json.size());

        if (json.at(index).type() != type)
        {
            diagnosticBag.Add(CreateUnexpectedPackagePropertyTypeError(
                fileBuffer,
                namePrefix + "[" + std::to_string(index) + "]",
                json.at(index).type(),
                type
            ));
            return { CreateDefaultValue(type), diagnosticBag };
        }

        return { json.at(index), diagnosticBag };
    }

    static auto DiagnoseUnexpectedProperties(
        const FileBuffer* const fileBuffer,
        const nlohmann::json& json,
        const std::string& namePrefix,
        std::vector<std::string> expectedNames
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnosticBag{};

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
                diagnosticBag.Add(CreateUnexpectedPackagePropertyWarning(
                    fileBuffer,
                    namePrefix.empty() ? name : (namePrefix + "." + name)
                ));
            }
        }

        return diagnosticBag;
    }

    static auto ParsePathMacroMap(
        const FileBuffer* const fileBuffer,
        const nlohmann::json& package
    ) -> Diagnosed<std::unordered_map<std::string, std::string>>
    {
        DiagnosticBag diagnosticBag{};

        const auto dgnPathMacros = GetOrCreateProperty(
            fileBuffer,
            package,
            {},
            Property::PathMacros,
            nlohmann::json::value_t::array
        );
        diagnosticBag.Add(dgnPathMacros);

        std::unordered_map<std::string, std::string> pathMacroMap{};
        for (size_t i = 0; i < dgnPathMacros.Unwrap().size(); i++)
        {
            const auto dgnPathMacro = GetOrCreateElement(
                fileBuffer,
                dgnPathMacros.Unwrap(),
                Property::PathMacros,
                i,
                nlohmann::json::value_t::object
            );
            diagnosticBag.Add(dgnPathMacro);

            const auto pathMacroPropertyNamePrefix =
                Property::PathMacros + "[" + std::to_string(i) + "]";

            diagnosticBag.Add(DiagnoseUnexpectedProperties(
                fileBuffer,
                dgnPathMacro.Unwrap(),
                pathMacroPropertyNamePrefix,
                {
                    Property::Name,
                    Property::Value,
                }
            ));

            const auto dgnPathMacroName = GetOrCreateProperty(
                fileBuffer,
                dgnPathMacro.Unwrap(),
                pathMacroPropertyNamePrefix,
                Property::Name,
                nlohmann::json::value_t::string
            );
            diagnosticBag.Add(dgnPathMacroName);

            const auto dgnPathMacroValue = GetOrCreateProperty(
                fileBuffer,
                dgnPathMacro.Unwrap(),
                pathMacroPropertyNamePrefix,
                Property::Value,
                nlohmann::json::value_t::string
            );
            diagnosticBag.Add(dgnPathMacroValue);

            std::string pathMacroName = dgnPathMacroName.Unwrap();
            Trim(pathMacroName);
            std::string pathMacroValue = dgnPathMacroValue.Unwrap();
            Trim(pathMacroValue);
            
            pathMacroMap[pathMacroName] = std::move(pathMacroValue);
        }

        return { pathMacroMap, diagnosticBag };
    }

    static auto ParseFiles(
        const FileBuffer* const fileBuffer,
        const nlohmann::json& package,
        const std::string& filesPropertyName
    ) -> Diagnosed<std::vector<std::string>>
    {
        DiagnosticBag diagnosticBag{};

        const auto dgnFilesArray = GetOrCreateProperty(
            fileBuffer,
            package,
            {},
            filesPropertyName,
            nlohmann::json::value_t::array
        );
        diagnosticBag.Add(dgnFilesArray);

        std::vector<std::string> files{};
        for (size_t i = 0; i < dgnFilesArray.Unwrap().size(); i++)
        {
            const auto dgnFile = GetOrCreateElement(
                fileBuffer,
                dgnFilesArray.Unwrap(),
                filesPropertyName,
                i,
                nlohmann::json::value_t::string
            );
            diagnosticBag.Add(dgnFile);

            files.push_back(dgnFile.Unwrap());
        }

        return { files, diagnosticBag };
    }

    static auto ReadFilePaths(
        std::vector<std::shared_ptr<const ISourceBuffer>>* const sourceBuffers,
        const FileBuffer* const fileBuffer,
        const std::vector<std::filesystem::path>& filePaths
    ) -> Diagnosed<std::vector<const FileBuffer*>>
    {
        DiagnosticBag diagnosticBag{};

        std::vector<const FileBuffer*> fileBuffers{};
        std::for_each(begin(filePaths), end(filePaths),
        [&](const std::filesystem::path& filePath)
        {
            const auto expFileBuffer = ReadFilePath(
                sourceBuffers,
                fileBuffer,
                filePath
            );
            diagnosticBag.Add(expFileBuffer);
            if (!expFileBuffer)
            {
                return;
            }

            fileBuffers.push_back(expFileBuffer.Unwrap());
        });

        return { fileBuffers, diagnosticBag };
    }

    static auto ParsePackage(
        std::vector<std::shared_ptr<const ISourceBuffer>>* const sourceBuffers,
        const FileBuffer* const fileBuffer
    ) -> Diagnosed<Package>
    {
        DiagnosticBag diagnosticBag{};

        const auto package = nlohmann::json::parse(fileBuffer->GetBuffer());

        diagnosticBag.Add(DiagnoseUnexpectedProperties(
            fileBuffer,
            package,
            {},
            {
                Property::Name,
                Property::PathMacros,
                Property::SourceFiles,
                Property::DependencyFiles,
            }
        ));

        auto dgnName = GetOrCreateProperty(
            fileBuffer,
            package,
            {},
            Property::Name,
            nlohmann::json::value_t::string
        );
        diagnosticBag.Add(dgnName);

        const auto dgnPathMacroMap = ParsePathMacroMap(
            fileBuffer,
            package
        );
        diagnosticBag.Add(dgnPathMacroMap);

        const auto dgnSourceFiles = ParseFiles(
            fileBuffer,
            package,
            Property::SourceFiles
        );
        diagnosticBag.Add(dgnSourceFiles);

        const auto dgnDependencyFiles = ParseFiles(
            fileBuffer,
            package,
            Property::DependencyFiles
        );
        diagnosticBag.Add(dgnDependencyFiles);

        const auto dgnSourceFilePaths = TransformFilePaths(
            fileBuffer,
            fileBuffer->GetPath(),
            dgnSourceFiles.Unwrap(),
            dgnPathMacroMap.Unwrap()
        );
        diagnosticBag.Add(dgnSourceFilePaths);

        const auto dgnDependencyFilePaths = TransformFilePaths(
            fileBuffer,
            fileBuffer->GetPath(),
            dgnDependencyFiles.Unwrap(),
            dgnPathMacroMap.Unwrap()
        );
        diagnosticBag.Add(dgnDependencyFilePaths);

        const auto dgnSourceFileBuffers = ReadFilePaths(
            sourceBuffers,
            fileBuffer,
            dgnSourceFilePaths.Unwrap()
        );
        diagnosticBag.Add(dgnSourceFileBuffers);

        return
        {
            Package
            {
                std::move(dgnName.Unwrap()),
                std::move(dgnSourceFileBuffers.Unwrap()),
                std::move(dgnDependencyFilePaths.Unwrap()),
            },
            diagnosticBag,
        };
    }

    auto Package::Parse(
        std::vector<std::shared_ptr<const ISourceBuffer>>* const sourceBuffers,
        const FileBuffer* const fileBuffer
    ) -> Expected<Package>
    {
        DiagnosticBag diagnosticBag{};

        try
        {
            const auto dgnPackage = ParsePackage(
                sourceBuffers,
                fileBuffer
            );
            diagnosticBag.Add(dgnPackage);

            return { dgnPackage.Unwrap(), diagnosticBag };
        }
        catch (const nlohmann::json::exception& exception)
        {
            return diagnosticBag.Add(CreateJsonError(
                fileBuffer,
                exception
            ));
        }
    }
}
