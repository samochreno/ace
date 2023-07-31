#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/All.hpp"

namespace Ace
{
    auto CreateMismatchedAccessModifierError(
        const SrcLocation& newSymbolNameLocation,
        const ISymbol* const originalSymbol,
        const AccessModifier newSymbolAccessModifier
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateSymbolRedefinitionError(
        const ISymbol* const redefinedSymbol
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateUnableToDeduceTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateUnableToDeduceTemplateArgError(
        const SrcLocation& srcLocation,
        const NormalTemplateParamTypeSymbol* const templateParam
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateTooManyTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateTemplateArgDeductionConflict(
        const SrcLocation& srcLocation,
        const NormalTemplateParamTypeSymbol* const templateParam,
        const ITypeSymbol* const deducedArg,
        const ITypeSymbol* const conflictingDeducedArg
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateUndefinedSymbolRefError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateAmbiguousSymbolRefError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateNonSelfScopedSymbolScopeAccessError(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateUndefinedTemplateInstanceRefError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateInaccessibleSymbolError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateIncorrectSymbolCategoryError(
        const SrcLocation& srcLocation,
        const SymbolCategory expectedCategory
    ) -> std::shared_ptr<const Diagnostic>;
}
