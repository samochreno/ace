#pragma once

#include <memory>
#include <string>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/All.hpp"

namespace Ace
{
    inline auto CreateAccessModifierString(
        const AccessModifier accessModifier
    ) -> std::string
    {
        switch (accessModifier)
        {
            case AccessModifier::Public:
            {
                return "public";
            }

            case AccessModifier::Private:
            {
                return "private";
            }
        }
    }

    inline auto CreateMismatchedAccessModifierError(
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

    inline auto CreateSymbolRedefinitionError(
        const ISymbol* const redefinedSymbol
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            redefinedSymbol->GetName().SrcLocation,
            "symbol redefinition"
        );
    }

    inline auto CreateUnableToDeduceTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "unable to deduce template arguments"
        );
    }

    inline auto CreateUnableToDeduceTemplateArgError(
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

    inline auto CreateTooManyTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "too many template arguments"
        );
    }

    inline auto CreateTemplateArgDeductionConflict(
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

    inline auto CreateUndefinedSymbolRefError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "undefined symbol reference"
        );
    }

    inline auto CreateAmbiguousSymbolRefError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "ambiguous symbol reference"
        );
    }

    inline auto CreateSymbolKindStringWithArticle(
        const SymbolKind symbolKind
    ) -> std::string
    {
        switch (symbolKind)
        {
            case SymbolKind::Module:
            {
                return "a module";
            }

            case SymbolKind::Struct:
            {
                return "a struct";
            }

            case SymbolKind::Label:
            {
                return "a label";
            }

            case SymbolKind::Function:
            {
                return "a function";
            }

            case SymbolKind::StaticVar:
            {
                return "a global variable";
            }

            case SymbolKind::InstanceVar:
            {
                return "a field";
            }

            case SymbolKind::LocalVar:
            {
                return "a local variable";
            }

            case SymbolKind::ParamVar:
            {
                return "a parameter";
            }

            case SymbolKind::FunctionTemplate:
            {
                return "a function template";
            }

            case SymbolKind::TypeTemplate:
            {
                return "a type template";
            }
            
            case SymbolKind::TypeAlias:
            {
                return "a type alias";
            }

            case SymbolKind::ImplTemplateParam:
            case SymbolKind::TemplateParam:
            {
                return "a template parameter";
            }

            case SymbolKind::ErrorType:
            {
                ACE_UNREACHABLE();
            }
        }
    }

    inline auto CreateNonSelfScopedSymbolScopeAccessError(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "scope access of " +
            CreateSymbolKindStringWithArticle(symbol->GetSymbolKind());

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );
    }

    inline auto CreateUndefinedTemplateInstanceRefError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "undefined template instance reference"
        );
    }

    inline auto CreateInaccessibleSymbolError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "inaccessible symbol"
        );
    }

    inline auto CreateSymbolCategoryStringWithArticle(
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

    inline auto CreateIncorrectSymbolCategoryError(
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