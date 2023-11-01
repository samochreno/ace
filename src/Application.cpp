#include "Application.hpp"

#include <memory>
#include <vector>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <map>
#include <llvm/Support/TargetSelect.h>

#include "Log.hpp"
#include "Compilation.hpp"
#include "FileBuffer.hpp"
#include "DynamicCastFilter.hpp"
#include "Scope.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "FunctionBlockBinding.hpp"
#include "Emitter.hpp"
#include "Syntaxes/All.hpp"
#include "Symbols/All.hpp"
#include "Semas/All.hpp"
#include "Diagnostic.hpp"
#include "GlueGeneration.hpp"
#include "Diagnoses/InvalidControlFlowDiagnosis.hpp"
#include "Diagnoses/LayoutCycleDiagnosis.hpp"
#include "Diagnoses/OrphanDiagnosis.hpp"
#include "Diagnoses/InvalidTraitImplDiagnosis.hpp"
#include "Diagnoses/OverlappingInherentImplDiagnosis.hpp"
#include "Diagnoses/ConcreteConstraintDiagnosis.hpp"

namespace Ace::Application
{
    static size_t IndentLevel = 3;

    static auto CreateIndent() -> std::string
    {
        return std::string(IndentLevel, ' ');
    }

    static auto ParseAST(
        Compilation* const compilation,
        const FileBuffer* const fileBuffer
    ) -> Expected<std::shared_ptr<const ModSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto tokens = diagnostics.Collect(LexTokens(fileBuffer));

        const auto optAST = diagnostics.Collect(
            ParseAST(fileBuffer, std::move(tokens))
        );
        if (!optAST.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected{ optAST.value(), std::move(diagnostics) };
    }

    auto CollectSyntaxes(
        const std::shared_ptr<const ISyntax>& ast
    ) -> std::vector<const ISyntax*>
    {
        auto syntaxes = ast->CollectChildren();
        syntaxes.push_back(ast.get());
        return syntaxes;
    }

    auto CreateAndDeclareSymbols(
        const std::vector<const ISyntax*>& syntaxes
    ) -> Diagnosed<std::vector<FunctionBlockBinding>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto declSyntaxes = DynamicCastFilter<const IDeclSyntax*>(syntaxes);

        std::sort(begin(declSyntaxes), end(declSyntaxes),
        [](const IDeclSyntax* const lhs, const IDeclSyntax* const rhs)
        {
            const auto lhsOrder = lhs->GetDeclOrder();
            const auto rhsOrder = rhs->GetDeclOrder();

            if (lhsOrder < rhsOrder)
            {
                return true;
            }

            if (lhsOrder > rhsOrder)
            {
                return false;
            }

            const auto lhsSuborder = lhs->GetDeclSuborder();
            const auto rhsSuborder = rhs->GetDeclSuborder();

            if (lhsSuborder < rhsSuborder)
            {
                return true;
            }

            if (lhsSuborder > rhsSuborder)
            {
                return false;
            }

            return false;
        });

        std::vector<FunctionBlockBinding> functionBlockBindings{};
        std::for_each(begin(declSyntaxes), end(declSyntaxes),
        [&](const IDeclSyntax* const declSyntax)
        {
            auto* const symbol = diagnostics.Collect(
                Scope::DeclareSymbol(declSyntax)
            );

            auto* const functionSyntax =
                dynamic_cast<const FunctionSyntax*>(declSyntax);
            if (!functionSyntax)
            {
                return;
            }

            auto* const functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
            ACE_ASSERT(functionSymbol);

            functionBlockBindings.emplace_back(
                functionSymbol,
                functionSyntax->GetBlock()
            );
        });

        return Diagnosed
        {
            std::move(functionBlockBindings),
            std::move(diagnostics),
        };
    }

    auto CreateVerifiedFunctionBlock(
        std::shared_ptr<const BlockStmtSema> block,
        ITypeSymbol* const functionTypeSymbol
    ) -> Diagnosed<std::shared_ptr<const BlockStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        block = diagnostics.Collect(
            block->CreateTypeChecked({ functionTypeSymbol })
        );
        block = block->CreateLowered({});

        return Diagnosed{ std::move(block), std::move(diagnostics) };
    }

    static auto CreateAndBindFunctionBlock(
        FunctionBlockBinding binding
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (!binding.OptBlockSyntax.has_value())
        {
            return Diagnosed<void>{ std::move(diagnostics) };
        }

        auto* const symbol = binding.Symbol;
        if (symbol->GetBlockSema().has_value())
        {
            return Diagnosed<void>{ std::move(diagnostics) };
        }

        const auto& blockSyntax = binding.OptBlockSyntax.value();

        const auto block = diagnostics.Collect(CreateVerifiedFunctionBlock(
            diagnostics.Collect(blockSyntax->CreateSema()),
            symbol->GetType()
        ));

        symbol->BindBlockSema(block);

        auto* const compilation = symbol->GetCompilation();

        const bool isVoid =
            symbol->GetType()->GetUnaliased() ==
            compilation->GetVoidTypeSymbol();

        if (!isVoid)
        {
            diagnostics.Collect(DiagnoseInvalidControlFlow(
                symbol->GetName().SrcLocation,
                ControlFlowGraph{ block->CreateControlFlowNodes() }
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto CreateAndBindFunctionBodies(
        const std::vector<FunctionBlockBinding>& bindings
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::for_each(begin(bindings), end(bindings),
        [&](const FunctionBlockBinding& binding)
        {
            diagnostics.Collect(CreateAndBindFunctionBlock(binding));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto CompileCompilation(
        Compilation* const compilation
    ) -> Expected<void>
    {
        auto diagnostics = DiagnosticBag::CreateGlobal();

        const auto& packageName = compilation->GetPackage().Name;
        const auto globalScope = compilation->GetGlobalScope();

        const auto& now = std::chrono::steady_clock::now;

        const auto timeBegin = now();
        const auto timeFrontendBegin = now();
        const auto timeParsingBegin = now();

        std::vector<std::shared_ptr<const ModSyntax>> asts{};
        std::for_each(
            begin(compilation->GetPackage().SrcFileBuffers),
            end  (compilation->GetPackage().SrcFileBuffers),
            [&](const FileBuffer* const srcFileBuffer)
            {
                const auto optAST = diagnostics.Collect(
                    ParseAST(compilation, srcFileBuffer)
                );
                if (!optAST.has_value())
                {
                    return;
                }

                asts.push_back(optAST.value());
            }
        );

        const auto timeParsingEnd = now();

        std::vector<const ISyntax*> syntaxes{};
        std::for_each(begin(asts), end(asts),
        [&](const std::shared_ptr<const ModSyntax>& ast)
        {
            const auto children = CollectSyntaxes(ast);
            syntaxes.insert(end(syntaxes), begin(children), end(children));
        });

        const auto timeDeclBegin = now();

        const auto functionBlockBindings = diagnostics.Collect(
            CreateAndDeclareSymbols(syntaxes)
        );
        diagnostics.Collect(globalScope->GetGenericInstantiator().InstantiateBodies(
            functionBlockBindings
        ));

        const auto timeDeclEnd = now();

        const auto timeBindingAndVerificationBegin = now();

        compilation->GetNatives().Verify();
        diagnostics.Collect(CreateAndBindFunctionBodies(functionBlockBindings));

        globalScope->GetGenericInstantiator().InstantiateReferencedMonos();
        GlueGeneration::GenerateAndBindGlue(compilation);
        globalScope->GetGenericInstantiator().InstantiateReferencedMonos();

        diagnostics.Collect(DiagnoseLayoutCycles(compilation));
        diagnostics.Collect(DiagnoseOrphans(compilation));
        diagnostics.Collect(DiagnoseInvalidTraitImpls(compilation));
        diagnostics.Collect(DiagnoseOverlappingInherentImpls(compilation));
        diagnostics.Collect(DiagnoseConcreteConstraints(compilation));

        const auto timeBindingAndVerificationEnd = now();

        const auto timeFrontendEnd = now();

        if (diagnostics.HasErrors())
        {
            return std::move(diagnostics);
        }

        const auto timeBackendBegin = now();

        Emitter emitter{ compilation };
        const auto optEmittingResult = diagnostics.Collect(emitter.Emit());
        if (!optEmittingResult.has_value())
        {
            return std::move(diagnostics);
        }

        const auto timeBackendEnd = now();
        
        const auto timeEnd = now();

        const auto createDurationString = [](std::chrono::nanoseconds duration) -> std::string
        {
            const auto minutes     = std::chrono::duration_cast<std::chrono::minutes>     (duration);
            const auto seconds     = std::chrono::duration_cast<std::chrono::seconds>     (duration -= minutes);
            const auto miliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration -= seconds);

            std::string value{};
            value += std::to_string(minutes.count());
            value += "m ";
            value += std::to_string(seconds.count());
            value += "s ";
            value += std::to_string(miliseconds.count());
            value += "ms";

            return value;
        };

        LogDebug << createDurationString(timeEnd - timeBegin) + " - total\n";
        LogDebug << createDurationString(timeFrontendEnd - timeFrontendBegin) + " - frontend\n";
        LogDebug << createDurationString(timeParsingEnd - timeParsingBegin) + " - frontend | parsing\n";
        LogDebug << createDurationString(timeDeclEnd - timeDeclBegin) + " - frontend | declaration\n";
        LogDebug << createDurationString(timeBindingAndVerificationEnd - timeBindingAndVerificationBegin) + " - frontend | binding and verification\n";
        LogDebug << createDurationString(timeBackendEnd - timeBackendBegin) + " - backend\n";
        LogDebug << createDurationString(optEmittingResult.value().Durations.GlueGeneration) + " - backend | glue generation\n";
        LogDebug << createDurationString(optEmittingResult.value().Durations.IREmitting) + " - backend | ir emitting\n";
        LogDebug << createDurationString(optEmittingResult.value().Durations.Analyses) + " - backend | analyses\n";
        LogDebug << createDurationString(optEmittingResult.value().Durations.LLC) + " - backend | llc\n";
        LogDebug << createDurationString(optEmittingResult.value().Durations.Clang) + " - backend | clang\n";

        return Void{ std::move(diagnostics) };
    }

    static auto Compile(
        std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
        const std::vector<std::string_view>& args
    ) -> Expected<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto compilationDiagnostics = DiagnosticBag::CreateGlobal();
        const auto optCompilation = compilationDiagnostics.Collect(
            Compilation::Parse(srcBuffers, args)
        );
        diagnostics.Add(std::move(compilationDiagnostics));
        if (!optCompilation.has_value())
        {
            return std::move(diagnostics);
        }

        Log << CreateIndent() << termcolor::bright_green << "Compiling ";
        Log << termcolor::reset;
        Log << optCompilation.value()->GetPackage().Name << "\n";
        IndentLevel++;

        const auto didCompile = diagnostics.Collect(
            CompileCompilation(optCompilation.value().get())
        );
        if (!didCompile || diagnostics.HasErrors())
        {
            Log << CreateIndent() << termcolor::bright_red << "Failed";
            Log << termcolor::reset << " to compile\n";
            IndentLevel++;

            return std::move(diagnostics);
        }

        Log << CreateIndent() << termcolor::bright_green << "Finished";
        Log << termcolor::reset << " compilation\n";
        IndentLevel++;

        return Void{ std::move(diagnostics) };
    }

    auto Main(const std::vector<std::string_view>& args) -> void
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();

        std::vector<std::shared_ptr<const ISrcBuffer>> srcBuffers{};

        const auto didCompile = Compile(
            &srcBuffers,
            { { "-oace/build" }, { "ace/package.json" } }
        );
        if (!didCompile)
        {
            return;
        }
    }
}
