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
    static size_t IndentLevel = 3;

    static auto CreateIndent() -> std::string
    {
        return std::string(IndentLevel, ' ');
    }

    static auto Compile(
        const Compilation* const t_compilation
    ) -> Expected<void>
    {
        DiagnosticBag diagnosticBag{};

        const auto& packageName = t_compilation->Package.Name;
        auto* const globalScope = t_compilation->GlobalScope.Unwrap().get();

        const auto& now = std::chrono::steady_clock::now;

        const auto timeStart = now();
        const auto timeFrontendStart = now();
        const auto timeParsingStart = now();

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

                t_compilation->GlobalDiagnosticBag->Add(dgnAST.GetDiagnosticBag());
                diagnosticBag.Add(dgnAST.GetDiagnosticBag());
                return dgnAST.Unwrap();
            }
        );

        const auto timeParsingEnd = now();

        const auto nodes = Core::GetAllNodes(begin(asts), end(asts));

        const auto timeSymbolCreationStart = now();

        ACE_TRY_VOID(Core::CreateAndDefineSymbols(t_compilation, nodes));
        ACE_TRY_VOID(Core::DefineAssociations(t_compilation, nodes));

        const auto timeSymbolCreationEnd = now();

        const auto templateSymbols = globalScope->CollectSymbolsRecursive<ITemplateSymbol>();
        t_compilation->TemplateInstantiator->SetSymbols(templateSymbols);
        ACE_TRY_VOID(t_compilation->TemplateInstantiator->InstantiatePlaceholderSymbols());

        const auto timeBindingAndVerificationStart = now();

        t_compilation->Natives->Initialize();

        std::vector<std::shared_ptr<const ModuleBoundNode>> boundASTs{};
        std::for_each(begin(asts), end(asts),
        [&](const std::shared_ptr<const ModuleNode>& t_ast)
        {
            const auto expBoundAST = Core::CreateBoundTransformedAndVerifiedAST(
                t_compilation,
                t_ast,
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
                }
            );
            if (!expBoundAST)
            {
                diagnosticBag.Add(expBoundAST);
                t_compilation->GlobalDiagnosticBag->Add(expBoundAST);
                return;
            }

            boundASTs.push_back(expBoundAST.Unwrap());
        });

        t_compilation->TemplateInstantiator->InstantiateSemanticsForSymbols();

        Core::GenerateAndBindGlue(t_compilation);

        const auto timeBindingAndVerificationEnd = now();

        ACE_TRY_VOID(Core::ValidateTypeSizes(t_compilation));

        const auto timeFrontendEnd = now();

        const auto timeBackendStart = now();

        Emitter emitter{ t_compilation };
        emitter.SetASTs(boundASTs);
        const auto emitterResult = emitter.Emit();

        const auto timeBackendEnd = now();
        
        const auto timeEnd = now();

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

        LogDebug << getFormattedDuration(timeEnd - timeStart) + " - total\n";
        LogDebug << getFormattedDuration(timeFrontendEnd - timeFrontendStart) + " - frontend\n";
        LogDebug << getFormattedDuration(timeParsingEnd - timeParsingStart) + " - frontend | parsing\n";
        LogDebug << getFormattedDuration(timeSymbolCreationEnd - timeSymbolCreationStart) + " - frontend | symbol creation\n";
        LogDebug << getFormattedDuration(timeBindingAndVerificationEnd - timeBindingAndVerificationStart) + " - frontend | binding and verification\n";
        LogDebug << getFormattedDuration(timeBackendEnd - timeBackendStart) + " - backend\n";
        LogDebug << getFormattedDuration(emitterResult.Durations.IREmitting) + " - backend | ir emitting\n";
        LogDebug << getFormattedDuration(emitterResult.Durations.Analyses) + " - backend | analyses\n";
        LogDebug << getFormattedDuration(emitterResult.Durations.LLC) + " - backend | llc\n";
        LogDebug << getFormattedDuration(emitterResult.Durations.Clang) + " - backend | clang\n";

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
        if (
            !expCompilation ||
            (diagnosticBag.GetSeverity() == DiagnosticSeverity::Error)
            )
        {
            GlobalDiagnosticBag{}.Add(expCompilation);
            return diagnosticBag;
        }

        Log << CreateIndent() << termcolor::bright_green << "Compiling ";
        Log << termcolor::reset;
        Log << expCompilation.Unwrap()->Package.Name << "\n";
        IndentLevel++;

        const auto expDidCompile = Compile(expCompilation.Unwrap().get());
        diagnosticBag.Add(expDidCompile);
        if (diagnosticBag.GetSeverity() == DiagnosticSeverity::Error)
        {
            Log << CreateIndent() << termcolor::bright_red << "Failed";
            Log << termcolor::reset << " to compile\n";
            IndentLevel++;

            return diagnosticBag;
        }


        Log << CreateIndent() << termcolor::bright_green << "Finished";
        Log << termcolor::reset << " compilation\n";
        IndentLevel++;

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
            return;
        }
    }
}
