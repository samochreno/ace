#include "Application.hpp"

#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <fmt/format.h>
#include <llvm/Support/TargetSelect.h>

#include "Log.hpp"
#include "Core.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "NativeSymbol.hpp"
#include "Emitter.hpp"
#include "Package.hpp"
#include "Node/Base.hpp"
#include "Node/Module.hpp"
#include "BoundNode/Base.hpp"
#include "BoundNode/Module.hpp"
#include "Symbol/Template/Base.hpp"

namespace Ace
{
    static auto CompileFiles(const std::string& t_packageName, const std::vector<std::filesystem::path>& t_filePaths, const std::vector<std::filesystem::path>& t_dependencyFilePaths) -> Expected<void>
    {
        ACE_LOG_INFO("Build started.");
        ACE_LOG_EMPTY();

        Emitter emitter{ t_packageName };

        const auto& now = std::chrono::steady_clock::now;

        const auto timeStart = now();
        
        const auto timeFrontendStart = now();

        const auto timeParsingStart = now();

        ACE_TRY(asts, TransformExpectedVector(t_filePaths, [&]
        (const std::filesystem::path& t_filePath) -> Expected<std::shared_ptr<const Node::Module>>
        {
            const std::ifstream fileStream{ t_filePath };
            ACE_TRY_ASSERT(fileStream.is_open());

            std::stringstream stringStream{};
            stringStream << fileStream.rdbuf();

            const std::string text = stringStream.str();

            return Core::ParseAST(t_packageName, text);
        }));

        const auto timeParsingEnd = now();

        const auto nodes = Core::GetAllNodes(begin(asts), end(asts));

        const auto timeSymbolCreationStart = now();

        ACE_TRY_VOID(Core::CreateAndDefineSymbols(nodes));
        ACE_TRY_VOID(Core::DefineAssociations(nodes));

        ACE_TRY_VOID(NativeSymbol::InitializeSymbols());

        const auto timeSymbolCreationEnd = now();

        const auto timeBindingAndVerificationStart = now();

        ACE_TRY(boundASTs, Core::CreateBoundAndVerifiedASTs(
            asts,
            [](const std::shared_ptr<const Node::Module>& t_ast) { return t_ast->CreateBound(); },
            [](const std::shared_ptr<const BoundNode::Module>& t_ast) { return t_ast->GetOrCreateTypeChecked({}); },
            [](const std::shared_ptr<const BoundNode::Module>& t_ast) { return t_ast->GetOrCreateLowered({}); },
            [](const std::shared_ptr<const BoundNode::Module>& t_ast) { return t_ast->GetOrCreateTypeChecked({}); }
        ));

        const auto templateSymbols = Scope::GetRoot()->CollectDefinedSymbolsRecursive<Symbol::Template::IBase>();
        bool didInstantiateSemantics = true;
        while (didInstantiateSemantics)
        {
            didInstantiateSemantics = false;

            std::for_each(begin(templateSymbols), end(templateSymbols), [&]
            (Symbol::Template::IBase* const t_templateSymbol)
            {
                if (t_templateSymbol->HasUninstantiatedSemanticsForSymbols())
                {
                    didInstantiateSemantics = true;
                    t_templateSymbol->InstantiateSemanticsForSymbols();
                }
            });
        }

        const auto timeBindingAndVerificationEnd = now();

        ACE_TRY_VOID(Core::AssertCanResolveTypeSizes());

        const auto timeFrontendEnd = now();

        const auto timeBackendStart = now();

        emitter.SetASTs(boundASTs);
        const auto emitterResult = emitter.Emit();

        const auto timeBackendEnd = now();
        
        const auto timeEnd = now();

        const auto getFormattedDuration = [](std::chrono::nanoseconds t_duration)
        {
            const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(t_duration);
            const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(t_duration -= minutes);
            const auto miliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t_duration -= seconds);

            std::string value{};

            value += fmt::format("{:0>2}", minutes.count());
            value += ":";
            value += fmt::format("{:0>2}", seconds.count());
            value += ".";
            value += fmt::format("{:0>3}", miliseconds.count());

            return value;
        };

        ACE_LOG_INFO("[" << getFormattedDuration(timeEnd - timeStart)                                               << "] Total");
        ACE_LOG_INFO("[" << getFormattedDuration(timeFrontendEnd - timeFrontendStart)                               << "] Frontend");
        ACE_LOG_INFO("[" << getFormattedDuration(timeParsingEnd - timeParsingStart)                                 << "] Frontend | Parsing");
        ACE_LOG_INFO("[" << getFormattedDuration(timeSymbolCreationEnd - timeSymbolCreationStart)                   << "] Frontend | Symbol Creation");
        ACE_LOG_INFO("[" << getFormattedDuration(timeBindingAndVerificationEnd - timeBindingAndVerificationStart)   << "] Frontend | Binding And Verification");
        ACE_LOG_INFO("[" << getFormattedDuration(timeBackendEnd - timeBackendStart)                                 << "] Backend");
        ACE_LOG_INFO("[" << getFormattedDuration(emitterResult.Durations.Analyses)                                  << "] Backend | Analyses");
        ACE_LOG_INFO("[" << getFormattedDuration(emitterResult.Durations.LLC)                                       << "] Backend | llc");
        ACE_LOG_INFO("[" << getFormattedDuration(emitterResult.Durations.Clang)                                     << "] Backend | clang");
        ACE_LOG_EMPTY();

        return ExpectedVoid;
    }

    auto Main() -> void
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();

        const std::filesystem::path packagePath{ "/home/samo/repos/ace/ace/Package.json" };
        const std::ifstream packageFileStream{ packagePath };
        ACE_ASSERT(packageFileStream);

        std::stringstream packageStringStream{};
        packageStringStream << packageFileStream.rdbuf();

        const auto package = Package::Parse(packageStringStream.str()).Unwrap();
        const auto didCompileFiles = CompileFiles(
            package.GetName(),
            package.GetFilePaths().CreatePaths(),
            package.GetDependencyFilePaths().CreatePaths()
        );

        if (didCompileFiles) 
        { 
            ACE_LOG_INFO("Build succeeded.");
        } 
        else 
        {
            ACE_LOG_WARNING("Build failed."); 
        }
    }
}
