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
    static auto Compile(
        const Compilation* const t_compilation
    ) -> Expected<void>
    {
        DiagnosticBag diagnosticBag{};

        const auto& packageName = t_compilation->Package.Name;
        auto* const globalScope = t_compilation->GlobalScope.Unwrap().get();

        const auto& now = std::chrono::steady_clock::now;

        const auto timeStart = now();
        Log(DiagnosticSeverity::Info, "build start");

        const auto timeFrontendStart = now();
        Log(DiagnosticSeverity::Info, "frontend start");

        const auto timeParsingStart = now();
        Log(DiagnosticSeverity::Info, "parsing start");

        std::vector<std::shared_ptr<const ModuleNode>> asts{};
        std::transform(
            begin(t_compilation->Package.SourceFileBuffers),
            end  (t_compilation->Package.SourceFileBuffers),
            back_inserter(asts),
            [&](const FileBuffer* const t_sourceFileBuffer)
            {
                const auto dgnAST = Core::ParseAST(
                    t_compilation,
                    t_sourceFileBuffer
                );
                diagnosticBag.Add(dgnAST.GetDiagnosticBag());
                return dgnAST.Unwrap();
            }
        );

        const auto timeParsingEnd = now();
        Log(DiagnosticSeverity::Info, "parsing success");

        const auto nodes = Core::GetAllNodes(begin(asts), end(asts));

        const auto timeSymbolCreationStart = now();
        Log(DiagnosticSeverity::Info, "symbol creation start");

        ACE_TRY_VOID(Core::CreateAndDefineSymbols(t_compilation, nodes));
        ACE_TRY_VOID(Core::DefineAssociations(t_compilation, nodes));

        const auto timeSymbolCreationEnd = now();
        Log(DiagnosticSeverity::Info, "symbol creation success");

        Log(
            DiagnosticSeverity::Info,
            "template placeholder symbols instantiation start"
        );
        const auto templateSymbols = globalScope->CollectSymbolsRecursive<ITemplateSymbol>();
        t_compilation->TemplateInstantiator->SetSymbols(templateSymbols);
        ACE_TRY_VOID(t_compilation->TemplateInstantiator->InstantiatePlaceholderSymbols());
        Log(
            DiagnosticSeverity::Info,
            "template placeholder symbols instantiation success"
        );

        const auto timeBindingAndVerificationStart = now();
        Log(DiagnosticSeverity::Info, "binding and verification start");

        Log(
            DiagnosticSeverity::Info,
            "native symbol initialization start"
        );
        t_compilation->Natives->Initialize();
        Log(
            DiagnosticSeverity::Info,
            "native symbol initialization success"
        );

        Log(
            DiagnosticSeverity::Info,
            "ast binding and verification start"
        );
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
        Log(
            DiagnosticSeverity::Info,
            "ast binding and verification success"
        );

        Log(
            DiagnosticSeverity::Info,
            "function symbol body binding start"
        );
        Core::BindFunctionSymbolsBodies(
            t_compilation,
            Core::GetAllNodes(begin(boundASTs), end(boundASTs))
        );
        Log(
            DiagnosticSeverity::Info,
            "function symbol body binding success"
        );

        Log(
            DiagnosticSeverity::Info,
            "template semantics instantiation start"
        );
        t_compilation->TemplateInstantiator->InstantiateSemanticsForSymbols();
        Log(
            DiagnosticSeverity::Info,
            "template semantics instantiation success"
        );

        Log(DiagnosticSeverity::Info, "glue generation start");
        Core::GenerateAndBindGlue(t_compilation);
        Log(DiagnosticSeverity::Info, "glue generation success");

        const auto timeBindingAndVerificationEnd = now();
        Log(DiagnosticSeverity::Info, "binding and verification success");

        ACE_TRY_VOID(Core::ValidateTypeSizes(t_compilation));

        const auto timeFrontendEnd = now();
        Log(DiagnosticSeverity::Info, "frontend success");

        const auto timeBackendStart = now();
        Log(DiagnosticSeverity::Info, "backend start");

        Emitter emitter{ t_compilation };
        emitter.SetASTs(boundASTs);
        const auto emitterResult = emitter.Emit();

        const auto timeBackendEnd = now();
        Log(DiagnosticSeverity::Info, "backend success");
        
        const auto timeEnd = now();
        Log(DiagnosticSeverity::Info, "build success");

        const auto getFormattedDuration = [](std::chrono::nanoseconds t_duration) -> std::string
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

        Log(
            DiagnosticSeverity::Info,
            getFormattedDuration(timeEnd - timeStart) + " - total"
        );
        Log(
            DiagnosticSeverity::Info,
            getFormattedDuration(timeFrontendEnd - timeFrontendStart) + " - frontend"
        );
        Log(
            DiagnosticSeverity::Info,
            getFormattedDuration(timeParsingEnd - timeParsingStart) + " - frontend | parsing"
        );
        Log(
            DiagnosticSeverity::Info,
            getFormattedDuration(timeSymbolCreationEnd - timeSymbolCreationStart) + " - frontend | symbol creation"
        );
        Log(
            DiagnosticSeverity::Info,
            getFormattedDuration(timeBindingAndVerificationEnd - timeBindingAndVerificationStart) + " - frontend | binding and verification"
        );
        Log(
            DiagnosticSeverity::Info,
            getFormattedDuration(timeBackendEnd - timeBackendStart) + " - backend"
        );
        Log(
            DiagnosticSeverity::Info,
            getFormattedDuration(emitterResult.Durations.IREmitting) + " - backend | ir emitting"
        );
        Log(
            DiagnosticSeverity::Info,
            getFormattedDuration(emitterResult.Durations.Analyses) + " - backend | analyses"
        );
        Log(
            DiagnosticSeverity::Info,
            getFormattedDuration(emitterResult.Durations.LLC) + " - backend | llc"
        );
        Log(
            DiagnosticSeverity::Info,
            getFormattedDuration(emitterResult.Durations.Clang) + " - backend | clang"
        );

        return Void{ diagnosticBag };
    }

    static auto Compile(
        std::vector<std::shared_ptr<const ISourceBuffer>>* const t_sourceBuffers,
        const std::vector<std::string_view>& t_args
    ) -> Expected<void>
    {
        DiagnosticBag diagnosticBag{};

        const auto expCompilation = Compilation::Parse(
            t_sourceBuffers,
            t_args
        );
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

        return Void{ diagnosticBag };
    }

    auto Main(const std::vector<std::string_view>& t_args) -> void
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();

        std::vector<std::shared_ptr<const ISourceBuffer>> sourceBuffers{};

        const auto expDidCompile = Compile(
            &sourceBuffers,
            std::vector<std::string_view>
            {
                { "-oace/build" },
                { "ace/package.json" },
            }
        );
        if (!expDidCompile)
        {
            Core::LogDiagnosticBag(expDidCompile.GetDiagnosticBag());
            return;
        }
    }
}
