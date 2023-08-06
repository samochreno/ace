#include "Application.hpp"

#include <memory>
#include <vector>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <llvm/Support/TargetSelect.h>

#include "Log.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
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
#include "CFA.hpp"
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
        Compilation* const compilation,
        const FileBuffer* const fileBuffer
    ) -> Expected<std::shared_ptr<const ModuleNode>>
    {
        DiagnosticBag diagnostics{};

        auto dgnTokens = LexTokens(fileBuffer);
        diagnostics.Add(dgnTokens.GetDiagnostics());

        const auto expAST = ParseAST(
            fileBuffer,
            std::move(dgnTokens.Unwrap())
        );
        diagnostics.Add(expAST);
        if (!expAST)
        {
            return diagnostics;
        }

        return Expected
        {
            expAST.Unwrap(),
            diagnostics,
        };
    }

    auto CreateAndDefineSymbols(
        Compilation* const compilation,
        const std::vector<const INode*>& nodes
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

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
            diagnostics.Add(Scope::DefineSymbol(symbolCreatableNode));
        });

        return Diagnosed<void>{ diagnostics };
    }

    auto DefineAssociations(
        Compilation* const compilation,
        const std::vector<const INode*>& nodes
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        const auto implNodes = DynamicCastFilter<const IImplNode*>(nodes);

        std::for_each(begin(implNodes), end(implNodes),
        [&](const IImplNode* const implNode)
        {
            diagnostics.Add(implNode->DefineAssociations());
        });

        return Diagnosed<void>{ diagnostics };
    }

    auto DiagnoseNotAllControlPathsReturn(
        Compilation* const compilation,
        const std::vector<const IBoundNode*>& nodes
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        const auto functionNodes =
            DynamicCastFilter<const FunctionBoundNode*>(nodes);

        std::for_each(begin(functionNodes), end(functionNodes),
        [&](const FunctionBoundNode* const functionNode)
        {
            if (
                functionNode->GetSymbol()->GetType()->GetUnaliased() == 
                compilation->GetNatives()->Void.GetSymbol()
                )
            {
                return;
            }

            if (!functionNode->GetBody().has_value())
            {
                return;
            }

            const CFAGraph cfaGraph
            {
                functionNode->GetBody().value()->CreateCFANodes()
            };

            diagnostics.Add(CFA(
                functionNode->GetSymbol()->GetName().SrcLocation,
                cfaGraph
            ));
        });

        return Diagnosed<void>{ diagnostics };
    }

    auto BindFunctionSymbolsBodies(
        Compilation* const compilation,
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

    static auto DiagnoseLayoutCycles(
        Compilation* const compilation
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        const auto typeSymbols =
            compilation->GetGlobalScope()->CollectSymbolsRecursive<ITypeSymbol>();

        std::for_each(begin(typeSymbols), end(typeSymbols),
        [&](const ITypeSymbol* const typeSymbol)
        {
            if (typeSymbol->IsError())
            {
                return;
            }

            auto* const templatableSymbol = dynamic_cast<const ITemplatableSymbol*>(
                typeSymbol
            );
            if (
                templatableSymbol &&
                templatableSymbol->IsTemplatePlaceholder()
                )
            {
                return;
            }

            const auto expSizeKind = typeSymbol->GetSizeKind();
            diagnostics.Add(expSizeKind);
        });

        return Diagnosed<void>{ diagnostics };
    }

    static auto DiagnoseUnsizedVarTypes(
        Compilation* const compilation
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        const auto varSymbols =
            compilation->GetGlobalScope()->CollectSymbolsRecursive<IVarSymbol>();

        std::for_each(begin(varSymbols), end(varSymbols),
        [&](IVarSymbol* const varSymbol)
        {
            auto* const typeSymbol = varSymbol->GetType();

            if (typeSymbol->IsError())
            {
                return;
            }

            const auto expSizeKind = typeSymbol->GetSizeKind();
            if (!expSizeKind)
            {
                return;
            }

            if (expSizeKind.Unwrap() == TypeSizeKind::Sized)
            {
                return;
            }

            diagnostics.Add(CreateUnsizedSymbolTypeError(varSymbol));
        });

        return Diagnosed<void>{ diagnostics };
    }

    static auto CompileCompilation(
        Compilation* const compilation
    ) -> Expected<void>
    {
        GlobalDiagnosticBag diagnostics{};

        const auto& packageName = compilation->GetPackage().Name;
        const auto globalScope = compilation->GetGlobalScope();

        const auto& now = std::chrono::steady_clock::now;

        const auto timeBegin = now();
        const auto timeFrontendBegin = now();
        const auto timeParsingBegin = now();

        std::vector<std::shared_ptr<const ModuleNode>> asts{};
        std::for_each(
            begin(compilation->GetPackage().SrcFileBuffers),
            end  (compilation->GetPackage().SrcFileBuffers),
            [&](const FileBuffer* const srcFileBuffer)
            {
                const auto expAST = ParseAST(
                    compilation,
                    srcFileBuffer
                );
                diagnostics.Add(expAST);
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
        diagnostics.Add(dgnCreateAndDefineSymbols);

        const auto dgnDefineAssociations = DefineAssociations(
            compilation,
            nodes
        );
        diagnostics.Add(dgnDefineAssociations);

        const auto timeSymbolCreationEnd = now();

        const auto templateSymbols = globalScope->CollectSymbolsRecursive<ITemplateSymbol>();
        compilation->GetTemplateInstantiator().SetSymbols(templateSymbols);
        diagnostics.Add(
            compilation->GetTemplateInstantiator().InstantiatePlaceholderSymbols()
        );

        const auto timeBindingAndVerificationBegin = now();

        compilation->GetNatives()->Initialize();

        std::vector<std::shared_ptr<const ModuleBoundNode>> boundASTs{};
        std::transform(begin(asts), end(asts), back_inserter(boundASTs),
        [&](const std::shared_ptr<const ModuleNode>& ast)
        {
            const auto dgnBoundAST = ast->CreateBound();
            diagnostics.Add(dgnBoundAST);
            return dgnBoundAST.Unwrap();
        });

        std::vector<std::shared_ptr<const ModuleBoundNode>> finalASTs{};
        std::for_each(begin(boundASTs), end(boundASTs),
        [&](const std::shared_ptr<const ModuleBoundNode>& boundAST)
        {
            const auto expFinalAST = CreateTransformedAndVerifiedAST(
                boundAST,
                [](const std::shared_ptr<const ModuleBoundNode>& ast)
                { 
                    return ast->CreateTypeChecked({});
                },
                [](const std::shared_ptr<const ModuleBoundNode>& ast)
                {
                    return ast->CreateLowered({});
                }
            );
            diagnostics.Add(expFinalAST);
            if (!expFinalAST)
            {
                return;
            }

            finalASTs.push_back(expFinalAST.Unwrap());
        });

        compilation->GetTemplateInstantiator().InstantiateSemanticsForSymbols();

        if (diagnostics.Unwrap().HasErrors())
        {
            return diagnostics.Unwrap();
        }

        diagnostics.Add(DiagnoseLayoutCycles(compilation));
        diagnostics.Add(DiagnoseUnsizedVarTypes(compilation));

        const auto timeBindingAndVerificationEnd = now();

        const auto timeFrontendEnd = now();

        const auto timeBackendBegin = now();

        GlueGeneration::GenerateAndBindGlue(compilation);

        Emitter emitter{ compilation };
        emitter.SetASTs(finalASTs);
        const auto emitterResult = emitter.Emit();

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
        LogDebug << createDurationString(timeSymbolCreationEnd - timeSymbolCreationBegin) + " - frontend | symbol creation\n";
        LogDebug << createDurationString(timeBindingAndVerificationEnd - timeBindingAndVerificationBegin) + " - frontend | binding and verification\n";
        LogDebug << createDurationString(timeBackendEnd - timeBackendBegin) + " - backend\n";
        LogDebug << createDurationString(emitterResult.Durations.IREmitting) + " - backend | ir emitting\n";
        LogDebug << createDurationString(emitterResult.Durations.Analyses) + " - backend | analyses\n";
        LogDebug << createDurationString(emitterResult.Durations.LLC) + " - backend | llc\n";
        LogDebug << createDurationString(emitterResult.Durations.Clang) + " - backend | clang\n";

        return Void{ diagnostics.Unwrap() };
    }

    static auto Compile(
        std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
        const std::vector<std::string_view>& args
    ) -> Expected<void>
    {
        DiagnosticBag diagnostics{};

        const auto expCompilation = Compilation::Parse(
            srcBuffers,
            args
        );
        diagnostics.Add(expCompilation);
        GlobalDiagnosticBag{}.Add(expCompilation);
        if (!expCompilation)
        {
            return diagnostics;
        }

        Log << CreateIndent() << termcolor::bright_green << "Compiling ";
        Log << termcolor::reset;
        Log << expCompilation.Unwrap()->GetPackage().Name << "\n";
        IndentLevel++;

        const auto expDidCompile = CompileCompilation(
            expCompilation.Unwrap().get()
        );
        diagnostics.Add(expDidCompile);
        if (!expDidCompile || diagnostics.HasErrors())
        {
            Log << CreateIndent() << termcolor::bright_red << "Failed";
            Log << termcolor::reset << " to compile\n";
            IndentLevel++;

            return diagnostics;
        }

        Log << CreateIndent() << termcolor::bright_green << "Finished";
        Log << termcolor::reset << " compilation\n";
        IndentLevel++;

        return Void{ diagnostics };
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
