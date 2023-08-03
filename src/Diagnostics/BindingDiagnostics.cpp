#include "Diagnostics/BindingDiagnostics.hpp"

#include <memory>
#include <string>

#include "Diagnostic.hpp"
#include "DiagnosticStringConversions.hpp"
#include "SrcLocation.hpp"
#include "Symbols/All.hpp"
#include "Op.hpp"
#include "Token.hpp"

namespace Ace
{
    auto CreateMismatchedAccessModifierError(
        const ISymbol* const originalSymbol,
        const SrcLocation& newSymbolNameLocation,
        const AccessModifier newSymbolAccessModifier
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            newSymbolNameLocation,
            "mismatched access modifier"
        );

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            originalSymbol->GetName().SrcLocation,
            "previous definition"
        );

        return group;
    }

    auto CreateSymbolRedefinitionError(
        const ISymbol* const originalSymbol,
        const ISymbol* const redefinedSymbol
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            redefinedSymbol->GetName().SrcLocation,
            "symbol redefinition"
        );

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            originalSymbol->GetName().SrcLocation,
            "previous definition"
        );

        return group;
    }

    auto CreateUnableToDeduceTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "unable to deduce template arguments"
        );

        return group;
    }

    auto CreateUnableToDeduceTemplateArgError(
        const SrcLocation& srcLocation,
        const NormalTemplateParamTypeSymbol* const templateParam
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        const std::string message =
            "unable to deduce template argument for parameter `" +
            templateParam->GetName().String + "`";

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        return group;
    }

    auto CreateTooManyTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "too many template arguments"
        );

        return group;
    }

    auto CreateTemplateArgDeductionConflict(
        const SrcLocation& srcLocation,
        const NormalTemplateParamTypeSymbol* const templateParam,
        const ITypeSymbol* const deducedArg,
        const ITypeSymbol* const conflictingDeducedArg
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        const std::string message =
            "template argument deduction conflict for parameter `" + 
            templateParam->GetName().String + "`: `" +
            deducedArg->GetName().String + "` and `" +
            conflictingDeducedArg->GetName().String + "`";

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        return group;
    }

    auto CreateUndefinedSymbolRefError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "undefined reference to symbol"
        );

        return group;
    }

    auto CreateAmbiguousSymbolRefError(
        const SrcLocation& srcLocation,
        const std::vector<ISymbol*>& candidateSymbols
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "ambiguous reference to symbol"
        );

        std::for_each(begin(candidateSymbols), end(candidateSymbols),
        [&](const ISymbol* const symbol)
        {
            group->Diagnostics.emplace_back(
                DiagnosticSeverity::Note,
                symbol->GetName().SrcLocation,
                "candidate symbol declaration"
            );
        });

        return group;
    }

    auto CreateScopeAccessOfNonSelfScopedSymbolError(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        const std::string message =
            "scope access of " +
            CreateSymbolKindStringWithArticle(symbol->GetKind());

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            symbol->GetName().SrcLocation,
            CreateSymbolKindString(symbol->GetKind()) + " declaration"
        );

        return group;
    }

    auto CreateInaccessibleSymbolError(
        const SrcLocation& srcLocation,
        ISymbol* const symbol
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "inaccessible " + CreateSymbolKindString(symbol->GetKind())
        );

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            symbol->GetName().SrcLocation,
            CreateSymbolKindString(symbol->GetKind()) + " declaration"
        );

        return group;
    }

    auto CreateIncorrectSymbolCategoryError(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol,
        const SymbolCategory expectedCategory
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        const std::string message = 
            "not " +
            CreateSymbolCategoryStringWithArticle(expectedCategory) +
            " symbol";

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            symbol->GetName().SrcLocation,
            "symbol declaration"
        );

        return group;
    }

    auto CreateMissingStructVarsError(
        const SrcLocation& srcLocation,
        StructTypeSymbol* const structSymbol,
        const std::vector<InstanceVarSymbol*>& missingVarSymbols
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        std::string message{};

        message += "missing field";
        if (missingVarSymbols.size() > 1)
        {
            message += "s";
        }

        message += " ";

        std::for_each(begin(missingVarSymbols), end(missingVarSymbols),
        [&](InstanceVarSymbol* const missingVarSymbol)
        {
            const bool isFirstMissingVar = missingVarSymbol == missingVarSymbols.front();
            const bool  isLastMissingVar = missingVarSymbol == missingVarSymbols.back();

            if (!isFirstMissingVar)
            {
                if (isLastMissingVar)
                {
                    message += " and";
                }
                else
                {
                    message += ",";
                }

                message += " ";
            }

            message += "`" + missingVarSymbol->GetName().String + "`";
        });

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            structSymbol->GetName().SrcLocation,
            "struct declaration"
        );

        return group;
    }

    auto CreateStructHasNoVarNamedError(
        StructTypeSymbol* const structSymbol,
        const Ident& fieldName
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        const std::string message =
            "`" + structSymbol->CreateSignature() +
            "` has no field named `" + fieldName.String + "`";

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            fieldName.SrcLocation,
            message
        );

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            structSymbol->GetName().SrcLocation,
            "`" + structSymbol->CreateSignature() +  "` declaration"
        );

        return group;
    }

    auto CreateStructVarInitializedMoreThanOnceError(
        const SrcLocation& srcLocation,
        const SrcLocation& previousSrcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "field initialized more than once"
        );

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            previousSrcLocation,
            "previous initialization"
        );

        return group;
    }

    auto CreateUndefinedUnaryOpRefError(
        const Op& op,
        ITypeSymbol* const type
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        const auto opToken = std::make_shared<const Token>(
            op.SrcLocation,
            op.TokenKind
        );

        const std::string message =
            "undefined reference to operator " + CreateOpString(opToken) +
            " for type `" + type->CreateSignature() + "`";

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            op.SrcLocation,
            message
        );

        return group;
    }

    auto CreateUndefinedBinaryOpRefError(
        const Op& op,
        ITypeSymbol* const lhsType,
        ITypeSymbol* const rhsType
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        const auto opToken = std::make_shared<const Token>(
            op.SrcLocation,
            op.TokenKind
        );

        const std::string message =
            "undefined reference to operator " + CreateOpString(opToken) +
            " for types `" + lhsType->CreateSignature() + "` and `" +
            rhsType->CreateSignature() + "`";

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            op.SrcLocation,
            message
        );

        return group;
    }

    auto CreateAmbiguousBinaryOpRefError(
        const Op& op,
        ITypeSymbol* const lhsType,
        ITypeSymbol* const rhsType
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        const auto opToken = std::make_shared<const Token>(
            op.SrcLocation,
            op.TokenKind
        );

        const std::string message =
            "ambiguous reference to operator " + CreateOpString(opToken) +
            " for types `" + lhsType->CreateSignature() + "` and `" +
            rhsType->CreateSignature() + "`";

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            op.SrcLocation,
            message
        );

        return group;
    }

    auto CreateExpectedFunctionError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const type
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        const std::string message =
            "expected a function, found `" + type->CreateSignature() + "`";

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        return group;
    }
}
