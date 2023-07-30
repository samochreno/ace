#include "Compilation.hpp"

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <filesystem>

#include <llvm/IR/LLVMContext.h>

#include "Diagnostic.hpp"
#include "Diagnostics/CompilationDiagnostics.hpp"
#include "SrcBuffer.hpp"
#include "FileBuffer.hpp"
#include "CLIArgBuffer.hpp"
#include "CLIArgParser.hpp"
#include "SrcLocation.hpp"
#include "Package.hpp"
#include "Natives.hpp"
#include "Scope.hpp"
#include "TemplateInstantiator.hpp"
#include "GlobalDiagnosticBag.hpp"
#include "Symbols/Types/ErrorTypeSymbol.hpp"

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
        std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
        const std::vector<std::string_view>& args
    ) -> Expected<std::unique_ptr<const Compilation>>
    {
        DiagnosticBag diagnostics{};

        auto self = std::make_unique<Compilation>();

        const auto cliArgBuffer = std::make_shared<const Ace::CLIArgBuffer>(
            self.get(),
            args
        );
        self->CLIArgBuffer = cliArgBuffer.get();
        srcBuffers->push_back(std::move(cliArgBuffer));

        const auto expCLIArgsParseResult = ParseCommandLineArgs(
            self->CLIArgBuffer,
            GetOptionDefinitions()
        );
        diagnostics.Add(expCLIArgsParseResult);
        if (!expCLIArgsParseResult)
        {
            return diagnostics;
        }

        const auto& positionalArgs =
            expCLIArgsParseResult.Unwrap().PositionalArgs;
        const auto& optionMap = expCLIArgsParseResult.Unwrap().OptionMap;

        if (positionalArgs.empty())
        {
            return diagnostics.Add(CreateMissingPackagePathArgError());
        }

        if (positionalArgs.size() > 1)
        {
            std::for_each(
                begin(positionalArgs),
                end  (positionalArgs),
                [&](const std::string_view positionalArg)
                {
                    const SrcLocation srcLocation
                    {
                        self->CLIArgBuffer,
                        begin(positionalArg),
                        end  (positionalArg),
                    };

                    diagnostics.Add(CreateMultiplePackagePathArgsError(
                        srcLocation
                    ));
                }
            );
        }

        const auto packagePath = positionalArgs.front();

        auto expPackageFileBuffer = FileBuffer::Read(
            self.get(),
            packagePath
        );
        diagnostics.Add(expPackageFileBuffer);
        if (!expPackageFileBuffer)
        {
            return diagnostics;
        }

        self->PackageFileBuffer = expPackageFileBuffer.Unwrap().get();
        srcBuffers->push_back(std::move(expPackageFileBuffer.Unwrap()));

        auto expPackage = Package::Parse(
            srcBuffers,
            self->PackageFileBuffer
        );
        diagnostics.Add(expPackage);
        if (!expPackage)
        {
            return diagnostics;
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
        self->Diagnostics = std::make_unique<GlobalDiagnosticBag>();

        auto errorTypeSymbol = std::make_unique<Ace::ErrorTypeSymbol>(
            self->GlobalScope.Unwrap()
        );
        self->ErrorTypeSymbol = self->GlobalScope.Unwrap()->DefineSymbol(
            std::move(errorTypeSymbol)
        ).Unwrap();
         
        if (diagnostics.HasErrors())
        {
            return diagnostics;
        }

        return
        {
            std::unique_ptr<const Compilation>{ std::move(self) },
            diagnostics,
        };
    }
}
