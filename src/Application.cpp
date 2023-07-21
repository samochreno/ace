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
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "FileBuffer.hpp"
#include "Compilation.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "SpecialIdentifier.hpp"
#include "Nodes/All.hpp"
#include "DynamicCastFilter.hpp"
#include "Scope.hpp"
#include "Symbols/All.hpp"
#include "BoundNodes/All.hpp"
#include "ControlFlowAnalysis.hpp"
#include "Emitter.hpp"
#include "GlueGeneration.hpp"

namespace Ace::Application
{
    static size_t IndentLevel = 3;

    static auto CreateIndent() -> std::string
    {
        return std::string(IndentLevel, ' ');
    }

    static auto ParseAST(
        const Compilation* const t_compilation,
    const FileBuffer* const t_fileBuffer
    ) -> Expected<std::shared_ptr<const ModuleNode>>
    {
        DiagnosticBag diagnosticBag{};

        Lexer lexer{ t_fileBuffer };
        auto dgnTokens = lexer.EatTokens();
        diagnosticBag.Add(dgnTokens.GetDiagnosticBag());

        const auto expAST = ParseAST(
            t_fileBuffer,
            std::move(dgnTokens.Unwrap())
        );
        diagnosticBag.Add(expAST);
        if (!expAST)
        {
            return diagnosticBag;
        }

        return Expected
        {
            expAST.Unwrap(),
            diagnosticBag,
        };
    }

    auto CreateAndDefineSymbols(
        const Compilation* const t_compilation,
        const std::vector<const INode*>& t_nodes
    ) -> Expected<void>
    {
        auto symbolCreatableNodes =
            DynamicCastFilter<const ISymbolCreatableNode*>(t_nodes);

        std::sort(begin(symbolCreatableNodes), end(symbolCreatableNodes),
        [](
            const ISymbolCreatableNode* const t_lhs,
            const ISymbolCreatableNode* const t_rhs
        )
        {
            const auto lhsCreationOrder =
                GetSymbolCreationOrder(t_lhs->GetSymbolKind());

            const auto rhsCreationOrder =
                GetSymbolCreationOrder(t_rhs->GetSymbolKind());

            if (lhsCreationOrder < rhsCreationOrder)
            {
                return true;
            }

            if (lhsCreationOrder > rhsCreationOrder)
            {
                return false;
            }

            const auto lhsKindSpecificCreationOrder =
                t_lhs->GetSymbolCreationSuborder();

            const auto rhsKindSpecificCreationOrder =
                t_rhs->GetSymbolCreationSuborder();

            if (lhsKindSpecificCreationOrder < rhsKindSpecificCreationOrder)
            {
                return true;
            }

            if (lhsKindSpecificCreationOrder > rhsKindSpecificCreationOrder)
            {
                return false;
            }

            return false;
        });

        ACE_TRY_VOID(TransformExpectedVector(symbolCreatableNodes,
        [](const ISymbolCreatableNode* const t_symbolCreatableNode) -> Expected<void>
        {
            ACE_TRY(symbol, Scope::DefineSymbol(t_symbolCreatableNode));
            return Void{};
        }));

        return Void{};
    }

    auto DefineAssociations(
        const Compilation* const t_compilation,
        const std::vector<const INode*>& t_nodes
    ) -> Expected<void>
    {
        const auto implNodes = DynamicCastFilter<const ImplNode*>(t_nodes);

        const auto didCreateAssociations = TransformExpectedVector(implNodes,
        [](const ImplNode* const t_implNode) -> Expected<void>
        {
            ACE_TRY_VOID(t_implNode->DefineAssociations());
            return Void{};
        });
        ACE_TRY_ASSERT(didCreateAssociations);

        return Void{};
    }

    auto ValidateControlFlow(
        const Compilation* const t_compilation,
        const std::vector<const IBoundNode*>& t_nodes
    ) -> Expected<void>
    {
        const auto functionNodes =
            DynamicCastFilter<const FunctionBoundNode*>(t_nodes);

        const bool didControlFlowAnalysisSucceed = std::find_if(
            begin(functionNodes), 
            end  (functionNodes),
            [&](const FunctionBoundNode* const t_functionNode)
            {
                if (
                    t_functionNode->GetSymbol()->GetType()->GetUnaliased() == 
                    t_compilation->Natives->Void.GetSymbol()
                    )
                {
                    return false;
                }

                if (!t_functionNode->GetBody().has_value())
                {
                    return false;
                }

                ControlFlowAnalysis controlFlowAnalysis
                {
                    t_functionNode->GetBody().value()
                };

                return controlFlowAnalysis.IsEndReachableWithoutReturn();
            }
        ) == end(functionNodes);
        ACE_TRY_ASSERT(didControlFlowAnalysisSucceed);

        return Void{};
    }

    auto BindFunctionSymbolsBodies(
        const Compilation* const t_compilation,
        const std::vector<const IBoundNode*>& t_nodes
    ) -> void
    {
        const auto functionNodes =
            DynamicCastFilter<const FunctionBoundNode*>(t_nodes);

        std::for_each(begin(functionNodes), end(functionNodes),
        [&](const FunctionBoundNode* const t_functionNode)
        {
            if (!t_functionNode->GetBody().has_value())
            {
                return;
            }

            t_functionNode->GetSymbol()->BindBody(
                t_functionNode->GetBody().value()
            );
        });
    }

    static auto ValidateTypeSizes(
        const Compilation* const t_compilation
    ) -> Expected<void>
    {
        const auto typeSymbols =
            t_compilation->GlobalScope.Unwrap()->CollectSymbolsRecursive<ITypeSymbol>();

        const bool didValidateTypeSizes = std::find_if_not(
            begin(typeSymbols), 
            end  (typeSymbols),
            [](const ITypeSymbol* const t_typeSymbol) -> bool
            {
                auto* const templatableSymbol = dynamic_cast<const ITemplatableSymbol*>(
                    t_typeSymbol
                );
                if (
                    templatableSymbol &&
                    templatableSymbol->IsTemplatePlaceholder()
                    )
                {
                    return true;
                }

                return t_typeSymbol->GetSizeKind();
            }
        ) == end(typeSymbols);
        ACE_TRY_ASSERT(didValidateTypeSizes);

        return Void{};
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
        std::for_each(
            begin(t_compilation->Package.SourceFileBuffers),
            end  (t_compilation->Package.SourceFileBuffers),
            [&](const FileBuffer* const t_sourceFileBuffer)
            {
                const auto expAST = ParseAST(
                    t_compilation,
                    t_sourceFileBuffer
                );
                t_compilation->GlobalDiagnosticBag->Add(expAST);
                diagnosticBag.Add(expAST);
                if (!expAST)
                {
                    return;
                }

                asts.push_back(expAST.Unwrap());
            }
        );

        const auto timeParsingEnd = now();

        const auto nodes = GetAllNodes(begin(asts), end(asts));

        const auto timeSymbolCreationStart = now();

        ACE_TRY_VOID(CreateAndDefineSymbols(t_compilation, nodes));
        ACE_TRY_VOID(DefineAssociations(t_compilation, nodes));

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
            const auto expBoundAST = CreateBoundTransformedAndVerifiedAST(
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

        GlueGeneration::GenerateAndBindGlue(t_compilation);

        const auto timeBindingAndVerificationEnd = now();

        ACE_TRY_VOID(ValidateTypeSizes(t_compilation));

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
        if (!expCompilation)
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
        if (!expDidCompile)
        {
            Log << CreateIndent() << termcolor::bright_red << "Failed";
            Log << termcolor::reset << " to compile\n";
            IndentLevel++;

            return diagnosticBag;
        }

        Log << CreateIndent() << termcolor::bright_green << "Finished";
        Log << termcolor::reset << " compilation\n";
        IndentLevel++;

        if (diagnosticBag.GetSeverity() == DiagnosticSeverity::Error)
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
            return;
        }
    }
}
