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
#include "Emitter.hpp"
#include "Node/All.hpp"
#include "BoundNode/All.hpp"
#include "Symbol/All.hpp"
#include "Compilation.hpp"

namespace Ace
{
    static auto Compile(const Compilation& t_compilation) -> Expected<void>
    {
        const auto& packageName = t_compilation.Package.Name;
        auto* const globalScope = t_compilation.GlobalScope.get();

        const auto& now = std::chrono::steady_clock::now;

        const auto timeStart = now();
        ACE_LOG_INFO("Build start");

        const auto timeFrontendStart = now();
        ACE_LOG_INFO("Frontend start");

        const auto timeParsingStart = now();
        ACE_LOG_INFO("Parsing start");

        ACE_TRY(asts, TransformExpectedVector(t_compilation.Package.FilePaths,
        [&](const std::filesystem::path& t_filePath) -> Expected<std::shared_ptr<const Node::Module>>
        {
            const std::ifstream fileStream{ t_filePath };
            ACE_TRY_ASSERT(fileStream.is_open());

            std::stringstream stringStream{};
            stringStream << fileStream.rdbuf();

            const std::string text = stringStream.str();

            return Core::ParseAST(t_compilation, text);
        }));

        const auto timeParsingEnd = now();
        ACE_LOG_INFO("Parsing success");

        const auto nodes = Core::GetAllNodes(begin(asts), end(asts));

        const auto timeSymbolCreationStart = now();
        ACE_LOG_INFO("Symbol creation start");

        ACE_TRY_VOID(Core::CreateAndDefineSymbols(t_compilation, nodes));
        ACE_TRY_VOID(Core::DefineAssociations(t_compilation, nodes));

        const auto timeSymbolCreationEnd = now();
        ACE_LOG_INFO("Symbol creation success");

        ACE_LOG_INFO("Template placeholder symbols instantiation start");
        const auto templateSymbols = globalScope->CollectDefinedSymbolsRecursive<Symbol::Template::IBase>();
        t_compilation.TemplateInstantiator->SetSymbols(templateSymbols);
        ACE_TRY_VOID(t_compilation.TemplateInstantiator->InstantiatePlaceholderSymbols());
        ACE_LOG_INFO("Template placeholder symbols instantiation success");

        const auto timeBindingAndVerificationStart = now();
        ACE_LOG_INFO("Binding and verification start");

        ACE_LOG_INFO("Native symbol initialization start");
        ACE_TRY_VOID(t_compilation.Natives->Initialize());
        ACE_LOG_INFO("Native symbol initialization success");

        ACE_LOG_INFO("AST binding and verification start");
        ACE_TRY(boundASTs, Core::CreateBoundTransformedAndVerifiedASTs(
            t_compilation,
            asts,
            [](const std::shared_ptr<const Node::Module>& t_ast)
            {
                return t_ast->CreateBound(); 
            },
            [](const std::shared_ptr<const BoundNode::Module>& t_ast)
            { 
                return t_ast->GetOrCreateTypeChecked({});
            },
            [](const std::shared_ptr<const BoundNode::Module>& t_ast)
            {
                return t_ast->GetOrCreateLowered({});
            },
            [](const std::shared_ptr<const BoundNode::Module>& t_ast)
            {
                return t_ast->GetOrCreateTypeChecked({});
            }
        ));
        ACE_LOG_INFO("AST binding and verification success");

        ACE_LOG_INFO("Function symbol body binding start");
        Core::BindFunctionSymbolsBodies(
            t_compilation,
            Core::GetAllNodes(begin(boundASTs), end(boundASTs))
        );
        ACE_LOG_INFO("Function symbol body binding success");

        ACE_LOG_INFO("Template semantics instantiation start");
        t_compilation.TemplateInstantiator->InstantiateSemanticsForSymbols();
        ACE_LOG_INFO("Template semantics instantiation success");

        ACE_LOG_INFO("Glue generation start");
        Core::GenerateAndBindGlue(t_compilation);
        ACE_LOG_INFO("Glue generation success");

        const auto timeBindingAndVerificationEnd = now();
        ACE_LOG_INFO("Binding and verification success");

        ACE_TRY_VOID(Core::AssertCanResolveTypeSizes(t_compilation));

        const auto timeFrontendEnd = now();
        ACE_LOG_INFO("Frontend success");

        const auto timeBackendStart = now();
        ACE_LOG_INFO("Backend start");

        Emitter emitter{ t_compilation };
        emitter.SetASTs(boundASTs);
        const auto emitterResult = emitter.Emit();

        const auto timeBackendEnd = now();
        ACE_LOG_INFO("Backend success");
        
        const auto timeEnd = now();
        ACE_LOG_INFO("Build success");

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

        ACE_LOG_INFO(getFormattedDuration(timeEnd - timeStart)                                               << " - Total");
        ACE_LOG_INFO(getFormattedDuration(timeFrontendEnd - timeFrontendStart)                               << " - Frontend");
        ACE_LOG_INFO(getFormattedDuration(timeParsingEnd - timeParsingStart)                                 << " - Frontend | Parsing");
        ACE_LOG_INFO(getFormattedDuration(timeSymbolCreationEnd - timeSymbolCreationStart)                   << " - Frontend | Symbol creation");
        ACE_LOG_INFO(getFormattedDuration(timeBindingAndVerificationEnd - timeBindingAndVerificationStart)   << " - Frontend | Binding and verification");
        ACE_LOG_INFO(getFormattedDuration(timeBackendEnd - timeBackendStart)                                 << " - Backend");
        ACE_LOG_INFO(getFormattedDuration(emitterResult.Durations.IREmitting)                                << " - Backend | IR emitting");
        ACE_LOG_INFO(getFormattedDuration(emitterResult.Durations.Analyses)                                  << " - Backend | Analyses");
        ACE_LOG_INFO(getFormattedDuration(emitterResult.Durations.LLC)                                       << " - Backend | llc");
        ACE_LOG_INFO(getFormattedDuration(emitterResult.Durations.Clang)                                     << " - Backend | clang");

        return ExpectedVoid;
    }

    static auto Compile(
        const std::vector<std::string>& t_args
    ) -> Expected<void>
    {
        ACE_TRY(compilation, Compilation::ParseAndVerify(t_args));
        ACE_TRY_VOID(Compile(*compilation.get()));

        return ExpectedVoid;
    }

    auto Main(const std::vector<std::string>& t_args) -> void
    {
        std::vector<std::string> args{};
        args.push_back("-o=ace/build");
        args.push_back("ace/package.json");

        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();

        const auto didCompile = Compile(args);
        if (!didCompile)
        {
            ACE_LOG_WARNING("Build fail");
            return;
        }
    }
}

