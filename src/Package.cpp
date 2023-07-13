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
    ) -> Diagnosed<std::vector<std::filesystem::path>>
    {
        DiagnosticBag diagnosticBag{};

        if (!std::filesystem::exists(t_directory.Path))
        {
            return { {}, diagnosticBag };
        }

        std::vector<std::filesystem::path> filePaths{};
        try
        {
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
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            diagnosticBag.Add(CreateFileSystemError(t_directory.Path, e));
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
        const FileBuffer* const t_fileBuffer,
        const std::string& t_part
    ) -> Diagnosed<ExpandedLastFilePathPartData>
    {
        DiagnosticBag diagnosticBag{};

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

            if (beforeExtension.at(1) != '*')
            {
                // TODO: Error
                return Recursiveness::NonRecursive;
            }

            if (beforeExtension.size() != 2)
            {
                // TODO: Error
            }

            return Recursiveness::Recursive;
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
            const std::filesystem::path& t_filePath
        ) : m_OptFilePath{ t_filePath }
        {
        }
        FilePathOrFilteredDirectory(
            const FilteredDirectory& t_directoryPath
        ) : m_OptFilteredDirectory{ t_directoryPath }
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
        const FileBuffer* const t_fileBuffer,
        const std::string& t_part,
        const std::unordered_map<std::string, std::string>& t_pathMacroMap
    ) -> Diagnosed<std::string>
    {
        DiagnosticBag diagnosticBag{};

        const bool isMacro = t_part.starts_with('$');
        if (!isMacro)
        {
            return { t_part, diagnosticBag };
        }

        const std::string pathMacro = t_part.substr(1);

        const auto macroValueIt = t_pathMacroMap.find(pathMacro);
        if (macroValueIt == end(t_pathMacroMap))
        {
            diagnosticBag.Add(CreateUndefinedReferenceToPackagePathMacroError(
                t_fileBuffer,
                pathMacro
            ));

            return { {}, diagnosticBag };
        }
        
        return { macroValueIt->second, diagnosticBag };
    }

    static auto ExpandFilePathParts(
        const FileBuffer* const t_fileBuffer,
        const std::vector<std::string>& t_filePathParts,
        const std::unordered_map<std::string, std::string>& t_pathMacroMap
    ) -> Diagnosed<FilePathOrFilteredDirectory>
    {
        DiagnosticBag diagnosticBag{};

        const auto dgnFirstFilePathPart = ExpandFirstFilePathPart(
            t_fileBuffer,
            t_filePathParts.front(),
            t_pathMacroMap
        );
        diagnosticBag.Add(dgnFirstFilePathPart);

        const auto dgnLastFilePathPartData = ExpandLastFilePathPart(
            t_fileBuffer,
            t_filePathParts.back()
        );
        diagnosticBag.Add(dgnLastFilePathPartData);

        std::string path = dgnFirstFilePathPart.Unwrap() + '/';
        std::for_each(
            begin(t_filePathParts) + 1,
            end  (t_filePathParts) - 1,
            [&](const std::string& t_part) { path += t_part + '/'; }
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
        const std::string& t_filePath
    ) -> Expected<std::vector<std::string>>
    {
        DiagnosticBag diagnosticBag{};

        const auto isPathSeparator = [](const char& t_character) -> bool
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

        if (!isPathSeparator(character))
        {
            parts.emplace_back(itBegin, end(t_filePath));
        }
        else
        {
            return diagnosticBag.Add(CreateFilePathEndsWithSeparatorError(
                t_filePath
            ));
        }

        return { parts, diagnosticBag };
    }

    static auto TransformFilePaths(
        const FileBuffer* const t_fileBuffer,
        const std::filesystem::path& t_packageFilePath,
        const std::vector<std::string>& t_filePaths,
        const std::unordered_map<std::string, std::string>& t_pathMacroMap
    ) -> Diagnosed<std::vector<std::filesystem::path>>
    {
        DiagnosticBag diagnosticBag{};

        std::vector<std::vector<std::string>> filePathsParts{};
        std::for_each(begin(t_filePaths), end(t_filePaths),
        [&](const std::string& t_filePath)
        {
            const auto expFilePathParts = SplitFilePath(t_filePath);
            diagnosticBag.Add(expFilePathParts);
            if (!expFilePathParts)
            {
                return;
            }

            filePathsParts.push_back(expFilePathParts.Unwrap());
        });

        const auto packageDirectoryPath = t_packageFilePath.parent_path();

        std::vector<std::filesystem::path> finalFilePaths{};
        std::for_each(begin(filePathsParts), end(filePathsParts),
        [&](const std::vector<std::string>& t_filePathParts)
        {
            const auto dgnFilePathOrFilteredDirectory = ExpandFilePathParts(
                t_fileBuffer,
                t_filePathParts,
                t_pathMacroMap
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

        return { expFileBuffer.Unwrap().get(), diagnosticBag };
    }

    static auto CreateDefaultValue(
        const nlohmann::json::value_t t_type
    ) -> nlohmann::json
    {
        switch (t_type)
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
        const FileBuffer* const t_fileBuffer,
        const nlohmann::json& t_json,
        const std::string& t_namePrefix,
        const std::string& t_name,
        const nlohmann::json::value_t t_type
    ) -> Diagnosed<nlohmann::json>
    {
        DiagnosticBag diagnosticBag{};

        const auto prefixedName = t_namePrefix.empty() ?
            t_name :
            (t_namePrefix + "." + t_name);

        if (!t_json.contains(t_name))
        {
            diagnosticBag.Add(CreateMissingPackagePropertyError(
                t_fileBuffer,
                prefixedName
            ));
            return { CreateDefaultValue(t_type), diagnosticBag };
        }

        if (t_json[t_name].type() != t_type)
        {
            diagnosticBag.Add(CreateUnexpectedPackagePropertyTypeError(
                t_fileBuffer,
                prefixedName,
                t_json[t_name].type(),
                t_type
            ));
            return { CreateDefaultValue(t_type), diagnosticBag };
        }

        return { t_json[t_name], diagnosticBag };
    }

    static auto GetOrCreateElement(
        const FileBuffer* const t_fileBuffer,
        const nlohmann::json& t_json,
        const std::string& t_namePrefix,
        const size_t t_index,
        const nlohmann::json::value_t t_type
    ) -> Diagnosed<nlohmann::json>
    {
        DiagnosticBag diagnosticBag{};

        ACE_ASSERT(t_index < t_json.size());

        if (t_json.at(t_index).type() != t_type)
        {
            diagnosticBag.Add(CreateUnexpectedPackagePropertyTypeError(
                t_fileBuffer,
                t_namePrefix + "[" + std::to_string(t_index) + "]",
                t_json.at(t_index).type(),
                t_type
            ));
            return { CreateDefaultValue(t_type), diagnosticBag };
        }

        return { t_json.at(t_index), diagnosticBag };
    }

    static auto DiagnoseUnexpectedProperties(
        const FileBuffer* const t_fileBuffer,
        const nlohmann::json& t_json,
        const std::string& t_namePrefix,
        std::vector<std::string> t_expectedNames
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnosticBag{};

        for (const auto& nameValuePair : t_json.items())
        {
            const auto& name  = nameValuePair.key();
            const auto& value = nameValuePair.value();

            const auto matchingNameIt = std::find_if(
                begin(t_expectedNames),
                end  (t_expectedNames),
                [&](const std::string& t_expectedName)
                {
                    return name == t_expectedName;
                }
            );
            if (matchingNameIt != end(t_expectedNames))
            {
                t_expectedNames.erase(matchingNameIt);
            }
            else
            {
                diagnosticBag.Add(CreateUnexpectedPackagePropertyWarning(
                    t_fileBuffer,
                    t_namePrefix.empty() ? name : (t_namePrefix + "." + name)
                ));
            }
        }

        return diagnosticBag;
    }

    static auto ParsePathMacroMap(
        const FileBuffer* const t_fileBuffer,
        const nlohmann::json& t_package
    ) -> Diagnosed<std::unordered_map<std::string, std::string>>
    {
        DiagnosticBag diagnosticBag{};

        const auto dgnPathMacros = GetOrCreateProperty(
            t_fileBuffer,
            t_package,
            {},
            Property::PathMacros,
            nlohmann::json::value_t::array
        );
        diagnosticBag.Add(dgnPathMacros);

        std::unordered_map<std::string, std::string> pathMacroMap{};
        for (size_t i = 0; i < dgnPathMacros.Unwrap().size(); i++)
        {
            const auto dgnPathMacro = GetOrCreateElement(
                t_fileBuffer,
                dgnPathMacros.Unwrap(),
                Property::PathMacros,
                i,
                nlohmann::json::value_t::object
            );
            diagnosticBag.Add(dgnPathMacro);

            const auto pathMacroPropertyNamePrefix =
                Property::PathMacros + "[" + std::to_string(i) + "]";

            diagnosticBag.Add(DiagnoseUnexpectedProperties(
                t_fileBuffer,
                dgnPathMacro.Unwrap(),
                pathMacroPropertyNamePrefix,
                {
                    Property::Name,
                    Property::Value,
                }
            ));

            const auto dgnPathMacroName = GetOrCreateProperty(
                t_fileBuffer,
                dgnPathMacro.Unwrap(),
                pathMacroPropertyNamePrefix,
                Property::Name,
                nlohmann::json::value_t::string
            );
            diagnosticBag.Add(dgnPathMacroName);

            const auto dgnPathMacroValue = GetOrCreateProperty(
                t_fileBuffer,
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
        const FileBuffer* const t_fileBuffer,
        const nlohmann::json& t_package,
        const std::string& t_filesPropertyName
    ) -> Diagnosed<std::vector<std::string>>
    {
        DiagnosticBag diagnosticBag{};

        const auto dgnFilesArray = GetOrCreateProperty(
            t_fileBuffer,
            t_package,
            {},
            t_filesPropertyName,
            nlohmann::json::value_t::array
        );
        diagnosticBag.Add(dgnFilesArray);

        std::vector<std::string> files{};
        for (size_t i = 0; i < dgnFilesArray.Unwrap().size(); i++)
        {
            const auto dgnFile = GetOrCreateElement(
                t_fileBuffer,
                dgnFilesArray.Unwrap(),
                t_filesPropertyName,
                i,
                nlohmann::json::value_t::string
            );
            diagnosticBag.Add(dgnFile);

            files.push_back(dgnFile.Unwrap());
        }

        return { files, diagnosticBag };
    }

    static auto ReadFilePaths(
        std::vector<std::shared_ptr<const ISourceBuffer>>* const t_sourceBuffers,
        const FileBuffer* const t_fileBuffer,
        const std::vector<std::filesystem::path>& t_filePaths
    ) -> Diagnosed<std::vector<const FileBuffer*>>
    {
        DiagnosticBag diagnosticBag{};

        std::vector<const FileBuffer*> fileBuffers{};
        std::for_each(begin(t_filePaths), end(t_filePaths),
        [&](const std::filesystem::path& t_filePath)
        {
            const auto expFileBuffer = ReadFilePath(
                t_sourceBuffers,
                t_fileBuffer,
                t_filePath
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
        std::vector<std::shared_ptr<const ISourceBuffer>>* const t_sourceBuffers,
        const FileBuffer* const t_fileBuffer
    ) -> Diagnosed<Package>
    {
        DiagnosticBag diagnosticBag{};

        const auto package = nlohmann::json::parse(t_fileBuffer->GetBuffer());

        diagnosticBag.Add(DiagnoseUnexpectedProperties(
            t_fileBuffer,
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
            t_fileBuffer,
            package,
            {},
            Property::Name,
            nlohmann::json::value_t::string
        );
        diagnosticBag.Add(dgnName);

        const auto dgnPathMacroMap = ParsePathMacroMap(
            t_fileBuffer,
            package
        );
        diagnosticBag.Add(dgnPathMacroMap);

        const auto dgnSourceFiles = ParseFiles(
            t_fileBuffer,
            package,
            Property::SourceFiles
        );
        diagnosticBag.Add(dgnSourceFiles);

        const auto dgnDependencyFiles = ParseFiles(
            t_fileBuffer,
            package,
            Property::DependencyFiles
        );
        diagnosticBag.Add(dgnDependencyFiles);

        const auto dgnSourceFilePaths = TransformFilePaths(
            t_fileBuffer,
            t_fileBuffer->GetPath(),
            dgnSourceFiles.Unwrap(),
            dgnPathMacroMap.Unwrap()
        );
        diagnosticBag.Add(dgnSourceFilePaths);

        const auto dgnDependencyFilePaths = TransformFilePaths(
            t_fileBuffer,
            t_fileBuffer->GetPath(),
            dgnDependencyFiles.Unwrap(),
            dgnPathMacroMap.Unwrap()
        );
        diagnosticBag.Add(dgnDependencyFilePaths);

        const auto dgnSourceFileBuffers = ReadFilePaths(
            t_sourceBuffers,
            t_fileBuffer,
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
        std::vector<std::shared_ptr<const ISourceBuffer>>* const t_sourceBuffers,
        const FileBuffer* const t_fileBuffer
    ) -> Expected<Package>
    {
        DiagnosticBag diagnosticBag{};

        try
        {
            const auto dgnPackage = ParsePackage(
                t_sourceBuffers,
                t_fileBuffer
            );
            diagnosticBag.Add(dgnPackage);

            return { dgnPackage.Unwrap(), diagnosticBag };
        }
        catch (const nlohmann::json::exception& exception)
        {
            return diagnosticBag.Add(CreateJsonError(
                t_fileBuffer,
                exception
            ));
        }
    }
}
