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
        const SrcLocation& newSymbolNameLocation,
        const ISymbol* const originalSymbol,
        const AccessModifier newSymbolAccessModifier
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "mismatched access modifier, previously defined as " +
            CreateAccessModifierString(newSymbolAccessModifier);

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            newSymbolNameLocation,
            message
        );
    }

    auto CreateSymbolRedefinitionError(
        const ISymbol* const redefinedSymbol
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            redefinedSymbol->GetName().SrcLocation,
            "symbol redefinition"
        );
    }

    auto CreateUnableToDeduceTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "unable to deduce template arguments"
        );
    }

    auto CreateUnableToDeduceTemplateArgError(
        const SrcLocation& srcLocation,
        const NormalTemplateParamTypeSymbol* const templateParam
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "unable to deduce template argument for parameter `" +
            templateParam->GetName().String + "`";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );
    }

    auto CreateTooManyTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "too many template arguments"
        );
    }

    auto CreateTemplateArgDeductionConflict(
        const SrcLocation& srcLocation,
        const NormalTemplateParamTypeSymbol* const templateParam,
        const ITypeSymbol* const deducedArg,
        const ITypeSymbol* const conflictingDeducedArg
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "template argument deduction conflict for parameter `" + 
            templateParam->GetName().String + "`: `" +
            deducedArg->GetName().String + "` and `" +
            conflictingDeducedArg->GetName().String + "`";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );
    }

    auto CreateUndefinedSymbolRefError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "undefined symbol reference"
        );
    }

    auto CreateAmbiguousSymbolRefError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "ambiguous symbol reference"
        );
    }

    auto CreateNonSelfScopedSymbolScopeAccessError(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "scope access of " +
            CreateSymbolKindStringWithArticle(symbol->GetKind());

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );
    }

    auto CreateUndefinedTemplateInstanceRefError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "undefined template instance reference"
        );
    }

    auto CreateInaccessibleSymbolError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "inaccessible symbol"
        );
    }

    

    auto CreateIncorrectSymbolCategoryError(
        const SrcLocation& srcLocation,
        const SymbolCategory expectedCategory
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message = 
            "symbol is not " +
            CreateSymbolCategoryStringWithArticle(expectedCategory);

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );
    }

    auto CreateMissingStructConstructionVarsError(
        const SrcLocation& srcLocation,
        StructTypeSymbol* const structSymbol,
        const std::vector<InstanceVarSymbol*>& missingVarSymbols
    ) -> std::shared_ptr<const Diagnostic>
    {
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

        message += " in construction of struct `";
        message += structSymbol->CreateSignature();
        message += "`";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );
    }

    auto CreateStructHasNoVarNamedError(
        StructTypeSymbol* const structSymbol,
        const Ident& fieldName
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "struct `" + structSymbol->CreateSignature() +
            "` has no field named `" + fieldName.String + "`";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            fieldName.SrcLocation,
            message
        );
    }

    auto CreateStructConstructionVarSpecifiedMoreThanOnceError(
        const SrcLocation& srcLocation,
        StructTypeSymbol* const structSymbol,
        InstanceVarSymbol* const varSymbol
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "field `" + varSymbol->GetName().String +
            "` specified more than once in construction of struct `" +
            structSymbol->CreateSignature() + "`";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );
    }

    auto CreateUndefinedUnaryOpRefError(
        const Op& op,
        ITypeSymbol* const type
    ) -> std::shared_ptr<const Diagnostic>
    {
        const auto opToken = std::make_shared<const Token>(
            op.SrcLocation,
            op.TokenKind
        );

        const std::string message =
            "undefined operator " + CreateOpString(opToken) +
            " reference for type `" + type->CreateSignature() + "`";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            op.SrcLocation,
            message
        );
    }

    auto CreateUndefinedBinaryOpRefError(
        const Op& op,
        ITypeSymbol* const lhsType,
        ITypeSymbol* const rhsType
    ) -> std::shared_ptr<const Diagnostic>
    {
        const auto opToken = std::make_shared<const Token>(
            op.SrcLocation,
            op.TokenKind
        );

        const std::string message =
            "undefined operator " + CreateOpString(opToken) +
            " reference for types `" + lhsType->CreateSignature() + "` and `" +
            rhsType->CreateSignature() + "`";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            op.SrcLocation,
            message
        );
    }

    auto CreateAmbiguousBinaryOpRefError(
        const Op& op,
        ITypeSymbol* const lhsType,
        ITypeSymbol* const rhsType
    ) -> std::shared_ptr<const Diagnostic>
    {
        const auto opToken = std::make_shared<const Token>(
            op.SrcLocation,
            op.TokenKind
        );

        const std::string message =
            "ambiguous operator " + CreateOpString(opToken) +
            " reference for types `" + lhsType->CreateSignature() + "` and `" +
            rhsType->CreateSignature() + "`";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            op.SrcLocation,
            message
        );
    }

    auto CreateExpectedFunctionError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const type
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "expected a function, found `" + type->CreateSignature() + "`";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );
    }
}
