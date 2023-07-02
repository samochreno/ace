#include "Application.hpp"

#include <memory>
#include <vector>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <fmt/format.h>
#include <llvm/Support/TargetSelect.h>

#include "Log.hpp"
#include "Core.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "Nodes/All.hpp"
#include "BoundNodes/All.hpp"
#include "Symbols/All.hpp"
#include "Compilation.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    static auto Compile(const Compilation* const t_compilation) -> Expected<void>
    {
        DiagnosticBag diagnosticBag{};

        const auto& packageName = t_compilation->Package.Name;
        auto* const globalScope = t_compilation->GlobalScope.Unwrap().get();

        const auto& now = std::chrono::steady_clock::now;

        const auto timeStart = now();
        ACE_LOG_INFO("Build start");

        const auto timeFrontendStart = now();
        ACE_LOG_INFO("Frontend start");

        const auto timeParsingStart = now();
        ACE_LOG_INFO("Parsing start");

        std::vector<std::shared_ptr<const ModuleNode>> asts{};
        std::transform(
            begin(t_compilation->Package.SourceFileBuffers),
            end  (t_compilation->Package.SourceFileBuffers),
            back_inserter(asts),
            [&](const FileBuffer& t_sourceFileBuffer)
            {
                const auto dgnAST = Core::ParseAST(
                    t_compilation,
                    &t_sourceFileBuffer
                );
                diagnosticBag.Add(dgnAST.GetDiagnosticBag());
                return dgnAST.Unwrap();
            }
        );

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
        const auto templateSymbols = globalScope->CollectSymbolsRecursive<ITemplateSymbol>();
        t_compilation->TemplateInstantiator->SetSymbols(templateSymbols);
        ACE_TRY_VOID(t_compilation->TemplateInstantiator->InstantiatePlaceholderSymbols());
        ACE_LOG_INFO("Template placeholder symbols instantiation success");

        const auto timeBindingAndVerificationStart = now();
        ACE_LOG_INFO("Binding and verification start");

        ACE_LOG_INFO("Native symbol initialization start");
        t_compilation->Natives->Initialize();
        ACE_LOG_INFO("Native symbol initialization success");

        ACE_LOG_INFO("AST binding and verification start");
        ACE_TRY(boundASTs, Core::CreateBoundTransformedAndVerifiedASTs(
            t_compilation,
            asts,
            [](const std::shared_ptr<const ModuleNode>& t_ast)
            {
                return t_ast->CreateBound(); 
            },
            [](const std::shared_ptr<const ModuleBoundNode>& t_ast)
            { 
                return t_ast->GetOrCreateTypeChecked({});
            },
            [](const std::shared_ptr<const ModuleBoundNode>& t_ast)
            {
                return t_ast->GetOrCreateLowered({});
            },
            [](const std::shared_ptr<const ModuleBoundNode>& t_ast)
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
        t_compilation->TemplateInstantiator->InstantiateSemanticsForSymbols();
        ACE_LOG_INFO("Template semantics instantiation success");

        ACE_LOG_INFO("Glue generation start");
        Core::GenerateAndBindGlue(t_compilation);
        ACE_LOG_INFO("Glue generation success");

        const auto timeBindingAndVerificationEnd = now();
        ACE_LOG_INFO("Binding and verification success");

        ACE_TRY_VOID(Core::ValidateTypeSizes(t_compilation));

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
            const auto minutes     = std::chrono::duration_cast<std::chrono::minutes>     (t_duration);
            const auto seconds     = std::chrono::duration_cast<std::chrono::seconds>     (t_duration -= minutes);
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

        return
        {
            Void,
            diagnosticBag,
        };
    }

    static auto Compile(
        const std::vector<std::string_view>& t_args
    ) -> Expected<void>
    {
        DiagnosticBag diagnosticBag{};

        const auto expCompilation = Compilation::Parse(t_args);
        diagnosticBag.Add(expCompilation);

        const auto hasCompilationError =
            expCompilation.GetDiagnosticBag().GetSeverity() ==
            DiagnosticSeverity::Error;

        if (!expCompilation || hasCompilationError)
        {
            return diagnosticBag;
        }

        const auto expDidCompile = Compile(expCompilation.Unwrap().get());
        diagnosticBag.Add(expDidCompile);
        if (!expDidCompile)
        {
            return diagnosticBag;
        }

        return
        {
            Void,
            diagnosticBag,
        };
    }

    auto Main(const std::vector<std::string_view>& t_args) -> void
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();

        const auto didCompile = Compile(std::vector<std::string_view>{
            { "-oace/build" },
            { "ace/package.json" },
        });
        if (!didCompile)
        {
            return;
        }
    }
}
