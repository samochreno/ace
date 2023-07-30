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
            return { {}, diagnostics };
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

        return { filePaths, diagnostics };
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
            return
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

        return
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
            return { part, diagnostics };
        }

        const std::string pathMacro = part.substr(1);

        const auto macroValueIt = pathMacroMap.find(pathMacro);
        if (macroValueIt == end(pathMacroMap))
        {
            diagnostics.Add(CreateUndefinedRefToPackagePathMacroError(
                fileBuffer,
                pathMacro
            ));

            return { {}, diagnostics };
        }
        
        return { macroValueIt->second, diagnostics };
    }

    static auto ExpandFilePathParts(
        const FileBuffer* const fileBuffer,
        const std::vector<std::string>& filePathParts,
        const std::unordered_map<std::string, std::string>& pathMacroMap
    ) -> Diagnosed<FilePathOrFilteredDirectory>
    {
        DiagnosticBag diagnostics{};

        const auto dgnFirstFilePathPart = ExpandFirstFilePathPart(
            fileBuffer,
            filePathParts.front(),
            pathMacroMap
        );
        diagnostics.Add(dgnFirstFilePathPart);

        const auto dgnLastFilePathPartData = ExpandLastFilePathPart(
            fileBuffer,
            filePathParts.back()
        );
        diagnostics.Add(dgnLastFilePathPartData);

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
            return { FilePathOrFilteredDirectory{ path }, diagnostics };
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

        return { parts, diagnostics };
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
            const auto expFilePathParts = SplitFilePath(filePath);
            diagnostics.Add(expFilePathParts);
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
            diagnostics.Add(dgnFilePathOrFilteredDirectory);

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
                diagnostics.Add(dgnDirectoryFilePaths);

                finalFilePaths.insert(
                    end(finalFilePaths),
                    begin(dgnDirectoryFilePaths.Unwrap()),
                    end  (dgnDirectoryFilePaths.Unwrap())
                );
            }
        });

        return { finalFilePaths, diagnostics };
    }

    static auto ReadFilePath(
        std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
        const FileBuffer* const fileBuffer,
        const std::filesystem::path& filePath
    ) -> Expected<const FileBuffer*>
    {
        DiagnosticBag diagnostics{};

        const auto expFileBuffer = FileBuffer::Read(
            fileBuffer->GetCompilation(),
            filePath
        );
        diagnostics.Add(expFileBuffer);
        if (!expFileBuffer)
        {
            return diagnostics;
        }

        srcBuffers->push_back(expFileBuffer.Unwrap());

        return { expFileBuffer.Unwrap().get(), diagnostics };
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
            return { CreateDefaultValue(type), diagnostics };
        }

        if (json[name].type() != type)
        {
            diagnostics.Add(CreateUnexpectedPackagePropertyTypeError(
                fileBuffer,
                prefixedName,
                json[name].type(),
                type
            ));
            return { CreateDefaultValue(type), diagnostics };
        }

        return { json[name], diagnostics };
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
            return { CreateDefaultValue(type), diagnostics };
        }

        return { json.at(index), diagnostics };
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

        return diagnostics;
    }

    static auto ParsePathMacroMap(
        const FileBuffer* const fileBuffer,
        const nlohmann::json& package
    ) -> Diagnosed<std::unordered_map<std::string, std::string>>
    {
        DiagnosticBag diagnostics{};

        const auto dgnPathMacros = GetOrCreateProperty(
            fileBuffer,
            package,
            {},
            Property::PathMacros,
            nlohmann::json::value_t::array
        );
        diagnostics.Add(dgnPathMacros);

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
            diagnostics.Add(dgnPathMacro);

            const auto pathMacroPropertyNamePrefix =
                Property::PathMacros + "[" + std::to_string(i) + "]";

            diagnostics.Add(DiagnoseUnexpectedProperties(
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
            diagnostics.Add(dgnPathMacroName);

            const auto dgnPathMacroValue = GetOrCreateProperty(
                fileBuffer,
                dgnPathMacro.Unwrap(),
                pathMacroPropertyNamePrefix,
                Property::Value,
                nlohmann::json::value_t::string
            );
            diagnostics.Add(dgnPathMacroValue);

            std::string pathMacroName = dgnPathMacroName.Unwrap();
            Trim(pathMacroName);
            std::string pathMacroValue = dgnPathMacroValue.Unwrap();
            Trim(pathMacroValue);
            
            pathMacroMap[pathMacroName] = std::move(pathMacroValue);
        }

        return { pathMacroMap, diagnostics };
    }

    static auto ParseFiles(
        const FileBuffer* const fileBuffer,
        const nlohmann::json& package,
        const std::string& filesPropertyName
    ) -> Diagnosed<std::vector<std::string>>
    {
        DiagnosticBag diagnostics{};

        const auto dgnFilesArray = GetOrCreateProperty(
            fileBuffer,
            package,
            {},
            filesPropertyName,
            nlohmann::json::value_t::array
        );
        diagnostics.Add(dgnFilesArray);

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
            diagnostics.Add(dgnFile);

            files.push_back(dgnFile.Unwrap());
        }

        return { files, diagnostics };
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
            const auto expFileBuffer = ReadFilePath(
                srcBuffers,
                fileBuffer,
                filePath
            );
            diagnostics.Add(expFileBuffer);
            if (!expFileBuffer)
            {
                return;
            }

            fileBuffers.push_back(expFileBuffer.Unwrap());
        });

        return { fileBuffers, diagnostics };
    }

    static auto ParsePackage(
        std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
        const FileBuffer* const fileBuffer
    ) -> Diagnosed<Package>
    {
        DiagnosticBag diagnostics{};

        const auto package = nlohmann::json::parse(fileBuffer->GetBuffer());

        diagnostics.Add(DiagnoseUnexpectedProperties(
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

        auto dgnName = GetOrCreateProperty(
            fileBuffer,
            package,
            {},
            Property::Name,
            nlohmann::json::value_t::string
        );
        diagnostics.Add(dgnName);

        const auto dgnPathMacroMap = ParsePathMacroMap(
            fileBuffer,
            package
        );
        diagnostics.Add(dgnPathMacroMap);

        const auto dgnSrcFiles = ParseFiles(
            fileBuffer,
            package,
            Property::SrcFiles
        );
        diagnostics.Add(dgnSrcFiles);

        const auto dgnDepFiles = ParseFiles(
            fileBuffer,
            package,
            Property::DepFiles
        );
        diagnostics.Add(dgnDepFiles);

        const auto dgnSrcFilePaths = TransformFilePaths(
            fileBuffer,
            fileBuffer->GetPath(),
            dgnSrcFiles.Unwrap(),
            dgnPathMacroMap.Unwrap()
        );
        diagnostics.Add(dgnSrcFilePaths);

        const auto dgnDepFilePaths = TransformFilePaths(
            fileBuffer,
            fileBuffer->GetPath(),
            dgnDepFiles.Unwrap(),
            dgnPathMacroMap.Unwrap()
        );
        diagnostics.Add(dgnDepFilePaths);

        const auto dgnSrcFileBuffers = ReadFilePaths(
            srcBuffers,
            fileBuffer,
            dgnSrcFilePaths.Unwrap()
        );
        diagnostics.Add(dgnSrcFileBuffers);

        return
        {
            Package
            {
                std::move(dgnName.Unwrap()),
                std::move(dgnSrcFileBuffers.Unwrap()),
                std::move(dgnDepFilePaths.Unwrap()),
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
            const auto dgnPackage = ParsePackage(
                srcBuffers,
                fileBuffer
            );
            diagnostics.Add(dgnPackage);

            return { dgnPackage.Unwrap(), diagnostics };
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
