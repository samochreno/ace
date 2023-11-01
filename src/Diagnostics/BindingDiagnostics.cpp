#include "Diagnostics/BindingDiagnostics.hpp"

#include <memory>
#include <string>

#include "Diagnostic.hpp"
#include "DiagnosticStringConversions.hpp"
#include "SrcLocation.hpp"
#include "Symbols/All.hpp"
#include "Op.hpp"
#include "Token.hpp"
#include "Keyword.hpp"

namespace Ace
{
    auto CreateMismatchedAccessModifierError(
        const ISymbol* const originalSymbol,
        const SrcLocation& newSymbolNameLocation,
        const AccessModifier newSymbolAccessModifier
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            newSymbolNameLocation,
            "mismatched access modifier"
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            originalSymbol->GetName().SrcLocation,
            "previous declaration"
        );

        return group;
    }

    auto CreateSymbolRedeclarationError(
        const ISymbol* const originalSymbol,
        const ISymbol* const redeclaredSymbol
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            redeclaredSymbol->GetName().SrcLocation,
            redeclaredSymbol->CreateTypeNoun().String + " redeclaration"
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            originalSymbol->GetName().SrcLocation,
            "previous declaration"
        );

        return group;
    }

    auto CreateStructFieldCausesCycleError(
        FieldVarSymbol* const fieldSymbol
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            fieldSymbol->GetName().SrcLocation,
            "field causes a cycle in struct layout"
        );

        return group;
    }

    auto CreateUnableToDeduceTypeArgsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "unable to deduce type arguments"
        );

        return group;
    }

    auto CreateUnableToDeduceTypeArgError(
        const SrcLocation& srcLocation,
        const TypeParamTypeSymbol* const typeParam
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unable to deduce argument for type parameter `" +
            typeParam->GetName().String + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        return group;
    }

    auto CreateTooManyTypeArgsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "too many type arguments"
        );

        return group;
    }

    auto CreateTypeArgDeductionConflict(
        const SrcLocation& srcLocation,
        const TypeParamTypeSymbol* const typeParam,
        const ITypeSymbol* const deducedArg,
        const ITypeSymbol* const conflictingDeducedArg
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "argument deduction conflict for type parameter `" + 
            typeParam->GetName().String + "`: `" +
            deducedArg->GetName().String + "` and `" +
            conflictingDeducedArg->GetName().String + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        return group;
    }

    auto CreateUndeclaredSymbolRefError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "undeclared symbol reference"
        );

        return group;
    }

    auto CreateAmbiguousSymbolRefError(
        const SrcLocation& srcLocation,
        const std::vector<ISymbol*>& candidateSymbols
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "ambiguous reference to symbol"
        );

        std::for_each(begin(candidateSymbols), end(candidateSymbols),
        [&](const ISymbol* const symbol)
        {
            group.Diagnostics.emplace_back(
                DiagnosticSeverity::Note,
                symbol->GetName().SrcLocation,
                "candidate symbol declaration"
            );
        });

        return group;
    }

    auto CreateScopeAccessOfNonBodyScopedSymbolError(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "scope access of " +
            symbol->CreateTypeNoun().CreateStringWithArticle();

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            symbol->GetName().SrcLocation,
            symbol->CreateTypeNoun().String + " declaration"
        );

        return group;
    }

    auto CreateInaccessibleSymbolError(
        const SrcLocation& srcLocation,
        ISymbol* const symbol
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "inaccessible " + symbol->CreateTypeNoun().String
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            symbol->GetName().SrcLocation,
            symbol->CreateTypeNoun().String + " declaration"
        );

        return group;
    }

    auto CreateIncorrectSymbolCategoryError(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol,
        const SymbolCategory expectedCategory
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message = 
            "not " +
            CreateSymbolCategoryStringWithArticle(expectedCategory) +
            " symbol";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            symbol->GetName().SrcLocation,
            "symbol declaration"
        );

        return group;
    }

    auto CreateMissingStructFieldsError(
        const SrcLocation& srcLocation,
        StructTypeSymbol* const structSymbol,
        const std::vector<FieldVarSymbol*>& missingFieldSymbols
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        std::string message{};

        message += "missing field";
        if (missingFieldSymbols.size() > 1)
        {
            message += "s";
        }

        message += " ";

        std::for_each(begin(missingFieldSymbols), end(missingFieldSymbols),
        [&](FieldVarSymbol* const fieldSymbol)
        {
            if (fieldSymbol != missingFieldSymbols.front())
            {
                if (fieldSymbol != missingFieldSymbols.back())
                {
                    message += ", ";
                }
                else
                {
                    message += " and ";
                }
            }

            message += "`" + fieldSymbol->GetName().String + "`";
        });

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            structSymbol->GetName().SrcLocation,
            "struct declaration"
        );

        return group;
    }

    auto CreateStructHasNoFieldNamedError(
        StructTypeSymbol* const structSymbol,
        const Ident& fieldName
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "`" + structSymbol->CreateDisplayName() +
            "` has no field named `" + fieldName.String + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            fieldName.SrcLocation,
            message
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            structSymbol->GetName().SrcLocation,
            "`" + structSymbol->CreateDisplayName() +  "` declaration"
        );

        return group;
    }

    auto CreateStructFieldInitializedMoreThanOnceError(
        const SrcLocation& srcLocation,
        const SrcLocation& previousSrcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "field initialized more than once"
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            previousSrcLocation,
            "previous initialization"
        );

        return group;
    }

    auto CreateUndeclaredUnaryOpError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const type
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "undeclared operator for type `" + type->CreateDisplayName() + "`"
        );

        return group;
    }

    auto CreateUndeclaredBinaryOpError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const lhsType,
        ITypeSymbol* const rhsType
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "undeclared operator for types `" + lhsType->CreateDisplayName() +
            "` and `" + rhsType->CreateDisplayName() + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        return group;
    }

    auto CreateAmbiguousBinaryOpRefError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const lhsType,
        ITypeSymbol* const rhsType
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "ambiguous operator for types `" + lhsType->CreateDisplayName() +
            "` and `" + rhsType->CreateDisplayName() + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        return group;
    }

    auto CreateExpectedFunctionError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const type
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "expected a function, found `" + type->CreateDisplayName() + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        return group;
    }

    auto CreateInherentImplOfForeignTypeError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "inherent implementation of foreign type"
        );

        return group;
    }

    auto CreateSelfReferenceInIncorrectContext(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "`" +
            std::string{ TokenKindToKeywordMap.at(TokenKind::SelfTypeKeyword) } + 
            "` reference in incorrect context";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        return group;
    }
}
