#include "Diagnostics/SymbolDiagnostics.hpp"

#include <memory>
#include <string>

#include "Diagnostic.hpp"
#include "DiagnosticStringConversions.hpp"
#include "SrcLocation.hpp"
#include "Symbols/All.hpp"

namespace Ace
{
    auto CreateMismatchedAccessModifierError(
        const SrcLocation& newSymbolNameLocation,
        const ISymbol* const originalSymbol,
        const AccessModifier newSymbolAccessModifier
    ) -> std::shared_ptr<const Diagnostic>
    {
        std::string message =
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

    auto CreateSymbolCategoryStringWithArticle(
        const SymbolCategory symbolCategory
    ) -> std::string
    {
        switch (symbolCategory)
        {
            case SymbolCategory::Instance:
            {
                return "an instance";
            }

            case SymbolCategory::Static:
            {
                return "static";
            }
        }
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
}
