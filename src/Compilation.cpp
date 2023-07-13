#include "Compilation.hpp"

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <filesystem>

#include <llvm/IR/LLVMContext.h>

#include "Diagnostics.hpp"
#include "SourceBuffer.hpp"
#include "FileBuffer.hpp"
#include "CLIArgBuffer.hpp"
#include "CLIArgParser.hpp"
#include "SourceLocation.hpp"
#include "Package.hpp"
#include "Natives.hpp"
#include "Scope.hpp"
#include "TemplateInstantiator.hpp"
#include "GlobalDiagnosticBag.hpp"

namespace Ace
{
    static const CLIOptionDefinition HelpOptionDefinition
    {
        std::string_view{ "h" },
        std::string_view{ "help" },
        CLIOptionKind::WithoutValue,
        std::nullopt,
    };

    static const CLIOptionDefinition OutputPathOptionDefinition
    {
        std::string_view{ "o" },
        std::string_view{ "output" },
        CLIOptionKind::WithValue,
        "build/",
    };

    static auto GetOptionDefinitions() -> std::vector<const CLIOptionDefinition*>
    {
        return
        {
            &HelpOptionDefinition,
            &OutputPathOptionDefinition,
        };
    }

    auto Compilation::Parse(
        std::vector<std::shared_ptr<const ISourceBuffer>>* const t_sourceBuffers,
        const std::vector<std::string_view>& t_args
    ) -> Expected<std::unique_ptr<const Compilation>>
    {
        DiagnosticBag diagnosticBag{};

        auto self = std::make_unique<Compilation>();

        const auto cliArgBuffer = std::make_shared<const Ace::CLIArgBuffer>(
            self.get(),
            t_args
        );
        self->CLIArgBuffer = cliArgBuffer.get();
        t_sourceBuffers->push_back(std::move(cliArgBuffer));

        const auto expCLIArgsParseResult = ParseCommandLineArgs(
            self->CLIArgBuffer,
            GetOptionDefinitions()
        );
        diagnosticBag.Add(expCLIArgsParseResult);
        if (!expCLIArgsParseResult)
        {
            return diagnosticBag;
        }

        const auto& positionalArgs =
            expCLIArgsParseResult.Unwrap().PositionalArgs;
        const auto& optionMap = expCLIArgsParseResult.Unwrap().OptionMap;

        if (positionalArgs.empty())
        {
            return diagnosticBag.Add(CreateMissingPackagePathArgError());
        }

        if (positionalArgs.size() > 1)
        {
            std::for_each(
                begin(positionalArgs),
                end  (positionalArgs),
                [&](const std::string_view t_positionalArg)
                {
                    const SourceLocation sourceLocation
                    {
                        self->CLIArgBuffer,
                        begin(t_positionalArg),
                        end  (t_positionalArg),
                    };

                    diagnosticBag.Add(CreateMultiplePackagePathArgsError(
                        sourceLocation
                    ));
                }
            );
        }

        const auto packagePath = positionalArgs.front();

        auto expPackageFileBuffer = FileBuffer::Read(
            self.get(),
            packagePath
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

        self->OutputPath = optionMap.at(
            &OutputPathOptionDefinition
        ).OptValue.value();

        if (
            !std::filesystem::exists(self->OutputPath) ||
            !std::filesystem::is_directory(self->OutputPath)
            )
        {
            std::filesystem::create_directories(self->OutputPath);
        }

        self->Natives = std::make_unique<Ace::Natives>(self.get());
        self->GlobalScope = { self.get() };
        self->TemplateInstantiator = std::make_unique<Ace::TemplateInstantiator>();
        self->LLVMContext = std::make_unique<llvm::LLVMContext>();
        self->GlobalDiagnosticBag = std::make_unique<Ace::GlobalDiagnosticBag>();

        if (diagnosticBag.GetSeverity() == DiagnosticSeverity::Error)
        {
            return diagnosticBag;
        }

        return
        {
            std::unique_ptr<const Compilation>{ std::move(self) },
            diagnosticBag,
        };
    }
}
