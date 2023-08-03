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
#include "ErrorSymbols.hpp"

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
    ) -> Expected<std::unique_ptr<Compilation>>
    {
        DiagnosticBag diagnostics{};

        auto self = std::make_unique<Compilation>();

        const auto cliArgBuffer = std::make_shared<const Ace::CLIArgBuffer>(
            self.get(),
            args
        );
        self->m_CLIArgBuffer = cliArgBuffer.get();
        srcBuffers->push_back(std::move(cliArgBuffer));

        const auto expCLIArgsParseResult = ParseCommandLineArgs(
            self->m_CLIArgBuffer,
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
            std::for_each(begin(positionalArgs), end(positionalArgs),
            [&](const std::string_view positionalArg)
            {
                const SrcLocation srcLocation
                {
                    self->m_CLIArgBuffer,
                    begin(positionalArg),
                    end  (positionalArg),
                };

                diagnostics.Add(CreateMultiplePackagePathArgsError(
                    srcLocation
                ));
            });
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

        self->m_PackageFileBuffer = expPackageFileBuffer.Unwrap().get();
        srcBuffers->push_back(std::move(expPackageFileBuffer.Unwrap()));

        auto expPackage = Package::Parse(
            srcBuffers,
            self->m_PackageFileBuffer
        );
        diagnostics.Add(expPackage);
        if (!expPackage)
        {
            return diagnostics;
        }

        self->m_Package = std::move(expPackage.Unwrap());

        self->m_OutputPath = optionMap.at(
            &OutputPathOptionDefinition
        ).OptValue.value();

        if (
            !std::filesystem::exists(self->m_OutputPath) ||
            !std::filesystem::is_directory(self->m_OutputPath)
            )
        {
            std::filesystem::create_directories(self->m_OutputPath);
        }

        self->m_Natives = std::make_unique<Ace::Natives>(self.get());
        self->m_GlobalScope = { self.get() };
        self->m_ErrorSymbols = Ace::ErrorSymbols{ self.get() };

        if (diagnostics.HasErrors())
        {
            return diagnostics;
        }

        return
        {
            std::unique_ptr<Compilation>{ std::move(self) },
            diagnostics,
        };
    }

    auto Compilation::GetCLIArgBuffer() const -> const Ace::CLIArgBuffer*
    {
        return m_CLIArgBuffer;
    }

    auto Compilation::GetPackageFileBuffer() const -> const FileBuffer*
    {
        return m_PackageFileBuffer;
    }

    auto Compilation::GetPackage() const -> const Ace::Package&
    {
        return m_Package;
    }

    auto Compilation::GetOutputPath() const -> const std::filesystem::path&
    {
        return m_OutputPath;
    }

    auto Compilation::GetNatives() const -> const Ace::Natives*
    {
        return m_Natives.get();
    }

    auto Compilation::GetNatives() -> Ace::Natives*
    {
        return m_Natives.get();
    }

    auto Compilation::GetGlobalScope() const -> const std::shared_ptr<Scope>&
    {
        return m_GlobalScope.Unwrap();
    }

    auto Compilation::GetTemplateInstantiator() const -> const Ace::TemplateInstantiator&
    {
        return m_TemplateInstantiator;
    }

    auto Compilation::GetTemplateInstantiator() -> Ace::TemplateInstantiator&
    {
        return m_TemplateInstantiator;
    }

    auto Compilation::GetLLVMContext() const -> const llvm::LLVMContext&
    {
        return m_LLVMContext;
    }

    auto Compilation::GetLLVMContext() -> llvm::LLVMContext&
    {
        return m_LLVMContext;
    }

    auto Compilation::GetDiagnostics() const -> const GlobalDiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto Compilation::GetDiagnostics() -> GlobalDiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto Compilation::GetErrorSymbols() const -> const Ace::ErrorSymbols&
    {
        return m_ErrorSymbols;
    }
}
