#include "Compilation.hpp"

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <filesystem>

#include <llvm/IR/LLVMContext.h>

#include "SourceBuffer.hpp"
#include "CommandLineArgBuffer.hpp"
#include "FileBuffer.hpp"
#include "SourceLocation.hpp"
#include "Diagnostics.hpp"
#include "Measured.hpp"
#include "Package.hpp"
#include "Natives.hpp"
#include "Scope.hpp"
#include "TemplateInstantiator.hpp"
#include "GlobalDiagnosticBag.hpp"

namespace Ace
{
    struct CompilationParseContext
    {
        const ISourceBuffer* SourceBuffer{};
        std::string_view Arg{};
        std::optional<std::string_view> OptNextArg{};
    };

    struct OptionDefinition
    {
        std::optional<std::string_view> ShortName{};
        std::string_view LongName{};
        bool DoesRequireValue{};
    };

    struct Option
    {
        const OptionDefinition* Definition{};
        std::optional<std::string_view> OptValue{};
    };

    using OptionMap = std::unordered_map<const OptionDefinition*, std::optional<std::string>>;

    struct StringBeginEndIterators
    {
        std::string::const_iterator Begin{};
        std::string::const_iterator End{};
    };

    static const OptionDefinition ErrorOptionDefinition
    {
        std::nullopt,
        std::string_view{ "" },
        true,
    };

    static const OptionDefinition PackagePathOptionDefinition
    {
        std::nullopt,
        std::string_view{ "" },
        true,
    };

    static const OptionDefinition HelpOptionDefinition
    {
        std::string_view{ "h" },
        std::string_view{ "help" },
        true,
    };

    static const OptionDefinition OutputPathOptionDefinition
    {
        std::string_view{ "o" },
        std::string_view{ "output" },
        true,
    };

    static const std::vector<const OptionDefinition*> OptionDefinitions
    {
        &ErrorOptionDefinition,
        &PackagePathOptionDefinition,
        &HelpOptionDefinition,
        &OutputPathOptionDefinition,
    };

    static auto DiagnoseMissingOptionName(
        const CompilationParseContext& t_context,
        const std::string_view t_name
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnosticBag{};

        if (!t_name.empty())
        {
            return diagnosticBag;
        }

        const SourceLocation sourceLocation
        {
            t_context.SourceBuffer,
            begin(t_context.Arg),
            end  (t_context.Arg),
        };

        return diagnosticBag.Add(CreateMissingCommandLineOptionNameError(
            sourceLocation
        ));
    }

    static auto DiagnoseMissingOrUnexpectedOptionValue(
        const CompilationParseContext& t_context,
        const OptionDefinition* const t_definition,
        const std::optional<std::string_view>& t_optValue
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnosticBag{};

        if (
            t_definition->DoesRequireValue &&
            !t_optValue.has_value()
            )
        {
            SourceLocation sourceLocation
            {
                t_context.SourceBuffer,
                begin(t_context.Arg),
                end  (t_context.Arg),
            };

            return diagnosticBag.Add(CreateMissingCommandLineOptionValueError(
                sourceLocation
            ));
        }

        if (
            !t_definition->DoesRequireValue &&
            t_optValue.has_value()
            )
        {
            SourceLocation sourceLocation
            {
                t_context.SourceBuffer,
                begin(t_optValue.value()),
                end  (t_optValue.value()),
            };

            return diagnosticBag.Add(CreateUnexpectedCommandLineOptionValueError(
                sourceLocation
            ));
        }

        return diagnosticBag;
    }

    static auto ParseLongOption(
        const CompilationParseContext& t_context
    ) -> Diagnosed<Measured<Option>>
    {
        DiagnosticBag diagnosticBag{};

        ACE_ASSERT(t_context.Arg.at(0) == '-');
        ACE_ASSERT(t_context.Arg.at(1) == '-');

        const std::string_view name
        {
            begin(t_context.Arg) + 2,
            std::find(begin(t_context.Arg), end(t_context.Arg), '='),
        };

        diagnosticBag.Add(DiagnoseMissingOptionName(
            t_context,
            name
        ));

        const auto matchedDefinitionIt = std::find_if(
            begin(OptionDefinitions),
            end  (OptionDefinitions),
            [&](const OptionDefinition* const t_optionDefinition)
            {
                return t_optionDefinition->LongName == name;
            }
        );
        if (matchedDefinitionIt == end(OptionDefinitions))
        {
            SourceLocation sourceLocation
            {
                t_context.SourceBuffer,
                begin(name),
                end  (name),
            };

            diagnosticBag.Add(CreateUnknownCommandLineOptionNameError(
                sourceLocation
            ));
        }

        const auto* const definition =
            (diagnosticBag.GetSeverity() == DiagnosticSeverity::Error) ?
            &ErrorOptionDefinition : 
            *matchedDefinitionIt;

        const bool isValueInCurrentString =
            end(name) != end(t_context.Arg);

        const auto optValue = [&]() -> std::optional<std::string_view>
        {
            if (isValueInCurrentString)
            {
                const std::string_view value
                {
                    end(name) + 1,
                    end(t_context.Arg),
                };

                if (value.empty())
                {
                    return std::nullopt;
                }

                return value;
            }

            if (!definition->DoesRequireValue)
            {
                return std::nullopt;
            }

            return t_context.OptNextArg;
        }();

        diagnosticBag.Add(DiagnoseMissingOrUnexpectedOptionValue(
            t_context,
            definition,
            optValue
        ));

        const Option option
        {
            definition,
            optValue,
        };

        const size_t length =
            (!optValue.has_value() || isValueInCurrentString) ? 1 : 2;

        return
        {
            Measured
            {
                option,
                length,
            },
            diagnosticBag,
        };
    }

    static auto ParseShortOption(
        const CompilationParseContext& t_context
    ) -> Diagnosed<Measured<Option>>
    {
        DiagnosticBag diagnosticBag{};

        ACE_ASSERT(t_context.Arg.at(0) == '-');
        ACE_ASSERT(t_context.Arg.at(1) != '-');

        const std::string_view name
        {
            begin(t_context.Arg) + 1,
            begin(t_context.Arg) + 2,
        };

        diagnosticBag.Add(DiagnoseMissingOptionName(
            t_context,
            name
        ));

        const auto matchedDefinitionIt = std::find_if(
            begin(OptionDefinitions),
            end  (OptionDefinitions),
            [&](const OptionDefinition* const t_optionDefinition)
            {
                return t_optionDefinition->ShortName == name;
            }
        );
        if (matchedDefinitionIt == end(OptionDefinitions))
        {
            SourceLocation sourceLocation
            {
                t_context.SourceBuffer,
                begin(name),
                end  (name),
            };

            diagnosticBag.Add(CreateUnknownCommandLineOptionNameError(
                sourceLocation
            ));
        }

        const auto* const definition =
            (diagnosticBag.GetSeverity() == DiagnosticSeverity::Error) ?
            &ErrorOptionDefinition : 
            *matchedDefinitionIt;

        const bool isValueInCurrentArg = t_context.Arg.size() > 2;

        const auto optValue = [&]() -> std::optional<std::string_view>
        {
            if (isValueInCurrentArg)
            {
                return std::string_view
                {
                    end(name),
                    end(t_context.Arg),
                };
            }

            if (!definition->DoesRequireValue)
            {
                return std::nullopt;
            }

            return t_context.OptNextArg;
        }();

        diagnosticBag.Add(DiagnoseMissingOrUnexpectedOptionValue(
            t_context,
            definition,
            optValue
        ));

        const Option option
        {
            definition,
            optValue,
        };

        const size_t length =
            (!optValue.has_value() || isValueInCurrentArg) ? 1 : 2;

        return
        {
            Measured
            {
                option,
                length,
            },
            diagnosticBag,
        };
    }

    static auto ParseOption(
        const CompilationParseContext& t_context
    ) -> Diagnosed<Measured<Option>>
    {
        ACE_ASSERT(t_context.Arg.at(0) == '-');

        return (t_context.Arg.at(1) != '-') ? 
            ParseShortOption(t_context) :
            ParseLongOption(t_context);
    }

    static auto CreateDefaultOptionMap() -> OptionMap
    {
        OptionMap optionMap{};

        optionMap[&PackagePathOptionDefinition] = std::nullopt;
        optionMap[&OutputPathOptionDefinition] = "./build/";

        return optionMap;
    }

    static auto ParseOptions(
        const CommandLineArgBuffer* const t_commandLineArgBuffer
    ) -> Diagnosed<OptionMap>
    {
        DiagnosticBag diagnosticBag{};

        std::vector<std::vector<std::string_view>::const_iterator> packagePathArgIts{};
        auto optionMap = CreateDefaultOptionMap();

        const auto& args = t_commandLineArgBuffer->GetArgs();

        auto argIt = begin(args);
        while (argIt != end(args))
        {
            const auto& arg = *argIt;

            if (!arg.starts_with('-'))
            {
                packagePathArgIts.push_back(argIt);
                ++argIt;
                continue;
            }

            const auto nextArgIt = argIt + 1;
            const bool hasNextArg = nextArgIt != end(args);

            const auto optNextArg = hasNextArg ?
                std::optional{ *nextArgIt } : 
                std::optional<std::string_view>{};

            const auto dgnOption = ParseOption({
                t_commandLineArgBuffer,
                arg,
                optNextArg
            });
            diagnosticBag.Add(dgnOption);

            optionMap[dgnOption.Unwrap().Value.Definition] =
                dgnOption.Unwrap().Value.OptValue;

            argIt += dgnOption.Unwrap().Length;
        }  

        if (packagePathArgIts.empty())
        {
            diagnosticBag.Add(CreateMissingPackagePathArgError());
        }
        else if (packagePathArgIts.size() == 1)
        {
            const std::string_view packagePath = *packagePathArgIts.front();
            optionMap[&PackagePathOptionDefinition] = packagePath;
        }
        else
        {
            std::for_each(begin(packagePathArgIts), end(packagePathArgIts),
            [&](const std::vector<std::string_view>::const_iterator t_packagePathArgIt)
            {
                SourceLocation sourceLocation
                {
                    t_commandLineArgBuffer,
                    begin(*t_packagePathArgIt),
                    end  (*t_packagePathArgIt),
                };

                diagnosticBag.Add(CreateMultiplePackagePathArgsError(
                    sourceLocation
                ));
            });
        }

        return
        {
            optionMap,
            diagnosticBag,
        };
    }

    auto Compilation::Parse(
        std::vector<std::shared_ptr<const ISourceBuffer>>* const t_sourceBuffers,
        const std::vector<std::string_view>& t_args
    ) -> Expected<std::unique_ptr<const Compilation>>
    {
        DiagnosticBag diagnosticBag{};

        auto self = std::make_unique<Compilation>();

        const auto commandLineArgBuffer = std::make_shared<const Ace::CommandLineArgBuffer>(
            self.get(),
            t_args
        );
        self->CommandLineArgBuffer = commandLineArgBuffer.get();
        t_sourceBuffers->push_back(std::move(commandLineArgBuffer));

        const auto dgnOptionMap = ParseOptions(self->CommandLineArgBuffer);
        diagnosticBag.Add(dgnOptionMap);

        const auto optPackagePath = dgnOptionMap.Unwrap().at(
            &PackagePathOptionDefinition
        );
        if (!optPackagePath.has_value())
        {
            return diagnosticBag;
        }

        auto expPackageFileBuffer = FileBuffer::Read(
            self.get(),
            optPackagePath.value()
        );
        diagnosticBag.Add(expPackageFileBuffer);
        if (!expPackageFileBuffer)
        {
            return diagnosticBag;
        }

        self->PackageFileBuffer = expPackageFileBuffer.Unwrap().get();
        t_sourceBuffers->push_back(std::move(expPackageFileBuffer.Unwrap()));

        auto expPackage = Package::Parse(
            t_sourceBuffers,
            self->PackageFileBuffer
        );
        diagnosticBag.Add(expPackage);
        if (!expPackage)
        {
            return diagnosticBag;
        }

        self->Package = std::move(expPackage.Unwrap());

        const auto optOutputPath = dgnOptionMap.Unwrap().at(
            &OutputPathOptionDefinition
        );
        if (optOutputPath.has_value())
        {
            if (
                !std::filesystem::exists(optOutputPath.value()) ||
                !std::filesystem::is_directory(optOutputPath.value())
                )
            {
                std::filesystem::create_directories(optOutputPath.value());
            }
        }

        self->OutputPath = optOutputPath.has_value() ?
            optOutputPath.value() :
            std::string{};

        self->Natives = std::make_unique<Ace::Natives>(self.get());
        self->GlobalScope = { self.get() };
        self->TemplateInstantiator = std::make_unique<Ace::TemplateInstantiator>();
        self->LLVMContext = std::make_unique<llvm::LLVMContext>();
        self->GlobalDiagnosticBag = std::make_unique<Ace::GlobalDiagnosticBag>();

        return
        {
            std::unique_ptr<const Compilation>{ std::move(self) },
            diagnosticBag,
        };
    }
}
