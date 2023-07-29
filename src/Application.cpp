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
#include "SpecialIdent.hpp"
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
        const Compilation* const compilation,
        const FileBuffer* const fileBuffer
    ) -> Expected<std::shared_ptr<const ModuleNode>>
    {
        DiagnosticBag diagnosticBag{};

        auto dgnTokens = LexTokens(fileBuffer);
        diagnosticBag.Add(dgnTokens.GetDiagnosticBag());

        const auto expAST = ParseAST(
            fileBuffer,
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
        const Compilation* const compilation,
        const std::vector<const INode*>& nodes
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnosticBag{};

        auto symbolCreatableNodes =
            DynamicCastFilter<const ISymbolCreatableNode*>(nodes);

        std::sort(begin(symbolCreatableNodes), end(symbolCreatableNodes),
        [](
            const ISymbolCreatableNode* const lhs,
            const ISymbolCreatableNode* const rhs
        )
        {
            const auto lhsCreationOrder =
                GetSymbolCreationOrder(lhs->GetSymbolKind());

            const auto rhsCreationOrder =
                GetSymbolCreationOrder(rhs->GetSymbolKind());

            if (lhsCreationOrder < rhsCreationOrder)
            {
                return true;
            }

            if (lhsCreationOrder > rhsCreationOrder)
            {
                return false;
            }

            const auto lhsKindSpecificCreationOrder =
                lhs->GetSymbolCreationSuborder();

            const auto rhsKindSpecificCreationOrder =
                rhs->GetSymbolCreationSuborder();

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

        std::for_each(begin(symbolCreatableNodes), end(symbolCreatableNodes),
        [&](const ISymbolCreatableNode* const symbolCreatableNode)
        {
            diagnosticBag.Add(Scope::DefineSymbol(symbolCreatableNode));
        });

        return Diagnosed<void>{ diagnosticBag };
    }

    auto DefineAssociations(
        const Compilation* const compilation,
        const std::vector<const INode*>& nodes
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnosticBag{};

        const auto implNodes = DynamicCastFilter<const IImplNode*>(nodes);

        std::for_each(begin(implNodes), end(implNodes),
        [&](const IImplNode* const implNode)
        {
            diagnosticBag.Add(implNode->DefineAssociations());
        });

        return Diagnosed<void>{ diagnosticBag };
    }

    auto ValidateControlFlow(
        const Compilation* const compilation,
        const std::vector<const IBoundNode*>& nodes
    ) -> Expected<void>
    {
        const auto functionNodes =
            DynamicCastFilter<const FunctionBoundNode*>(nodes);

        const bool didControlFlowAnalysisSucceed = std::find_if(
            begin(functionNodes), 
            end  (functionNodes),
            [&](const FunctionBoundNode* const functionNode)
            {
                if (
                    functionNode->GetSymbol()->GetType()->GetUnaliased() == 
                    compilation->Natives->Void.GetSymbol()
                    )
                {
                    return false;
                }

                if (!functionNode->GetBody().has_value())
                {
                    return false;
                }

                ControlFlowAnalysis controlFlowAnalysis
                {
                    functionNode->GetBody().value()
                };

                return controlFlowAnalysis.IsEndReachableWithoutReturn();
            }
        ) == end(functionNodes);
        ACE_TRY_ASSERT(didControlFlowAnalysisSucceed);

        return Void{};
    }

    auto BindFunctionSymbolsBodies(
        const Compilation* const compilation,
        const std::vector<const IBoundNode*>& nodes
    ) -> void
    {
        const auto functionNodes =
            DynamicCastFilter<const FunctionBoundNode*>(nodes);

        std::for_each(begin(functionNodes), end(functionNodes),
        [&](const FunctionBoundNode* const functionNode)
        {
            if (!functionNode->GetBody().has_value())
            {
                return;
            }

            functionNode->GetSymbol()->BindBody(
                functionNode->GetBody().value()
            );
        });
    }

    static auto ValidateTypeSizes(
        const Compilation* const compilation
    ) -> Expected<void>
    {
        const auto typeSymbols =
            compilation->GlobalScope.Unwrap()->CollectSymbolsRecursive<ITypeSymbol>();

        const bool didValidateTypeSizes = std::find_if_not(
            begin(typeSymbols), 
            end  (typeSymbols),
            [](const ITypeSymbol* const typeSymbol) -> bool
            {
                auto* const templatableSymbol = dynamic_cast<const ITemplatableSymbol*>(
                    typeSymbol
                );
                if (
                    templatableSymbol &&
                    templatableSymbol->IsTemplatePlaceholder()
                    )
                {
                    return true;
                }

                return typeSymbol->GetSizeKind();
            }
        ) == end(typeSymbols);
        ACE_TRY_ASSERT(didValidateTypeSizes);

        return Void{};
    }

    static auto Compile(const Compilation* const compilation) -> Expected<void>
    {
        DiagnosticBag diagnosticBag{};

        const auto& packageName = compilation->Package.Name;
        auto* const globalScope = compilation->GlobalScope.Unwrap().get();

        const auto& now = std::chrono::steady_clock::now;

        const auto timeBegin = now();
        const auto timeFrontendBegin = now();
        const auto timeParsingBegin = now();

        std::vector<std::shared_ptr<const ModuleNode>> asts{};
        std::for_each(
            begin(compilation->Package.SrcFileBuffers),
            end  (compilation->Package.SrcFileBuffers),
            [&](const FileBuffer* const srcFileBuffer)
            {
                const auto expAST = ParseAST(
                    compilation,
                    srcFileBuffer
                );
                compilation->GlobalDiagnosticBag->Add(expAST);
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

        const auto timeSymbolCreationBegin = now();

        const auto dgnCreateAndDefineSymbols = CreateAndDefineSymbols(
            compilation,
            nodes
        );
        compilation->GlobalDiagnosticBag->Add(dgnCreateAndDefineSymbols);
        diagnosticBag.Add(dgnCreateAndDefineSymbols);

        const auto dgnDefineAssociations = DefineAssociations(
            compilation,
            nodes
        );
        compilation->GlobalDiagnosticBag->Add(dgnDefineAssociations);
        diagnosticBag.Add(dgnDefineAssociations);

        const auto timeSymbolCreationEnd = now();

        const auto templateSymbols = globalScope->CollectSymbolsRecursive<ITemplateSymbol>();
        compilation->TemplateInstantiator->SetSymbols(templateSymbols);
        diagnosticBag.Add(
            compilation->TemplateInstantiator->InstantiatePlaceholderSymbols()
        );

        const auto timeBindingAndVerificationBegin = now();

        compilation->Natives->Initialize();

        std::vector<std::shared_ptr<const ModuleBoundNode>> boundASTs{};
        std::for_each(begin(asts), end(asts),
        [&](const std::shared_ptr<const ModuleNode>& ast)
        {
            const auto expBoundAST = CreateBoundTransformedAndVerifiedAST(
                ast,
                [](const std::shared_ptr<const ModuleNode>& ast)
                {
                    return ast->CreateBound(); 
                },
                [](const std::shared_ptr<const ModuleBoundNode>& ast)
                { 
                    return ast->GetOrCreateTypeChecked({});
                },
                [](const std::shared_ptr<const ModuleBoundNode>& ast)
                {
                    return ast->GetOrCreateLowered({});
                }
            );
            if (!expBoundAST)
            {
                diagnosticBag.Add(expBoundAST);
                compilation->GlobalDiagnosticBag->Add(expBoundAST);
                return;
            }

            boundASTs.push_back(expBoundAST.Unwrap());
        });

        compilation->TemplateInstantiator->InstantiateSemanticsForSymbols();

        if (diagnosticBag.HasErrors())
        {
            return diagnosticBag;
        }

        GlueGeneration::GenerateAndBindGlue(compilation);

        const auto timeBindingAndVerificationEnd = now();

        ACE_TRY_VOID(ValidateTypeSizes(compilation));

        const auto timeFrontendEnd = now();

        const auto timeBackendBegin = now();

        Emitter emitter{ compilation };
        emitter.SetASTs(boundASTs);
        const auto emitterResult = emitter.Emit();

        const auto timeBackendEnd = now();
        
        const auto timeEnd = now();

        const auto getFormattedDuration = [](std::chrono::nanoseconds duration) -> std::string
        {
            const auto minutes     = std::chrono::duration_cast<std::chrono::minutes>     (duration);
            const auto seconds     = std::chrono::duration_cast<std::chrono::seconds>     (duration -= minutes);
            const auto miliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration -= seconds);

            std::string value{};

            value += fmt::format("{:0>2}", minutes.count());
            value += ":";
            value += fmt::format("{:0>2}", seconds.count());
            value += ".";
            value += fmt::format("{:0>3}", miliseconds.count());

            return value;
        };

        LogDebug << getFormattedDuration(timeEnd - timeBegin) + " - total\n";
        LogDebug << getFormattedDuration(timeFrontendEnd - timeFrontendBegin) + " - frontend\n";
        LogDebug << getFormattedDuration(timeParsingEnd - timeParsingBegin) + " - frontend | parsing\n";
        LogDebug << getFormattedDuration(timeSymbolCreationEnd - timeSymbolCreationBegin) + " - frontend | symbol creation\n";
        LogDebug << getFormattedDuration(timeBindingAndVerificationEnd - timeBindingAndVerificationBegin) + " - frontend | binding and verification\n";
        LogDebug << getFormattedDuration(timeBackendEnd - timeBackendBegin) + " - backend\n";
        LogDebug << getFormattedDuration(emitterResult.Durations.IREmitting) + " - backend | ir emitting\n";
        LogDebug << getFormattedDuration(emitterResult.Durations.Analyses) + " - backend | analyses\n";
        LogDebug << getFormattedDuration(emitterResult.Durations.LLC) + " - backend | llc\n";
        LogDebug << getFormattedDuration(emitterResult.Durations.Clang) + " - backend | clang\n";

        return Void{ diagnosticBag };
    }

    static auto Compile(
        std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
        const std::vector<std::string_view>& args
    ) -> Expected<void>
    {
        DiagnosticBag diagnosticBag{};

        const auto expCompilation = Compilation::Parse(
            srcBuffers,
            args
        );
        diagnosticBag.Add(expCompilation);
        GlobalDiagnosticBag{}.Add(expCompilation);
        if (!expCompilation)
        {
            return diagnosticBag;
        }

        Log << CreateIndent() << termcolor::bright_green << "Compiling ";
        Log << termcolor::reset;
        Log << expCompilation.Unwrap()->Package.Name << "\n";
        IndentLevel++;

        const auto expDidCompile = Compile(expCompilation.Unwrap().get());
        diagnosticBag.Add(expDidCompile);
        if (!expDidCompile || diagnosticBag.HasErrors())
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

    auto Main(const std::vector<std::string_view>& args) -> void
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();

        std::vector<std::shared_ptr<const ISrcBuffer>> srcBuffers{};

        const auto expDidCompile = Compile(
            &srcBuffers,
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
