#include "Compilation.hpp"

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <filesystem>

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
#include "Symbols/Types/VoidTypeSymbol.hpp"
#include "ErrorSymbols.hpp"

namespace Ace
{
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
            &OutputPathOptionDefinition,
        };
    }

    auto Compilation::Parse(
        std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
        const std::vector<std::string_view>& args
    ) -> Expected<std::unique_ptr<Compilation>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto self = std::make_unique<Compilation>();

        const auto cliArgBuffer = std::make_shared<const Ace::CLIArgBuffer>(
            self.get(),
            args
        );
        self->m_CLIArgBuffer = cliArgBuffer.get();
        srcBuffers->push_back(std::move(cliArgBuffer));

        const auto optCLIArgsParseResult = diagnostics.Collect(ParseCommandLineArgs(
            self->m_CLIArgBuffer,
            GetOptionDefinitions()
        ));
        if (!optCLIArgsParseResult.has_value())
        {
            return std::move(diagnostics);
        }

        const auto& positionalArgs =
            optCLIArgsParseResult.value().PositionalArgs;
        const auto& optionMap = optCLIArgsParseResult.value().OptionMap;

        if (positionalArgs.empty())
        {
            diagnostics.Add(CreateMissingPackagePathArgError());
            return std::move(diagnostics);
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

        auto optPackageFileBuffer = diagnostics.Collect(FileBuffer::Read(
            self.get(),
            packagePath
        ));
        if (!optPackageFileBuffer.has_value())
        {
            return std::move(diagnostics);
        }

        self->m_PackageFileBuffer = optPackageFileBuffer.value().get();
        srcBuffers->push_back(std::move(optPackageFileBuffer.value()));

        auto optPackage = diagnostics.Collect(Package::Parse(
            srcBuffers,
            self->m_PackageFileBuffer
        ));
        if (!optPackage.has_value())
        {
            return std::move(diagnostics);
        }

        self->m_Package = std::move(optPackage.value());

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

        self->m_GlobalScope = { self.get() };

        self->m_PackageBodyScope =
            self->GetGlobalScope()->GetOrCreateChild(self->m_Package.Name);

        auto ownedVoidTypeSymbol = std::make_unique<VoidTypeSymbol>(
            self->GetGlobalScope()
        );
        self->m_VoidTypeSymbol = DiagnosticBag::CreateNoError().Collect(
            self->GetGlobalScope()->DeclareSymbol(std::move(ownedVoidTypeSymbol))
        );

        self->m_ErrorSymbols = Ace::ErrorSymbols{ self.get() };
        self->m_Natives = std::make_unique<const Ace::Natives>(self.get());

        if (diagnostics.HasErrors())
        {
            return std::move(diagnostics);
        }

        return
        {
            std::unique_ptr<Compilation>{ std::move(self) },
            std::move(diagnostics),
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

    auto Compilation::GetGlobalScope() const -> const std::shared_ptr<Scope>&
    {
        return m_GlobalScope.Unwrap();
    }

    auto Compilation::GetPackageBodyScope() const -> const std::shared_ptr<Scope>&
    {
        return m_PackageBodyScope;
    }

    auto Compilation::GetVoidTypeSymbol() const -> ITypeSymbol*
    {
        return m_VoidTypeSymbol;
    }

    auto Compilation::GetErrorSymbols() const -> const Ace::ErrorSymbols&
    {
        return m_ErrorSymbols;
    }

    auto Compilation::GetNatives() const -> const Ace::Natives&
    {
        return *m_Natives.get();
    }
}
