#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/All.hpp"
#include "Op.hpp"

namespace Ace
{
    auto CreateMismatchedAccessModifierError(
        const ISymbol* const originalSymbol,
        const SrcLocation& newSymbolNameLocation,
        const AccessModifier newSymbolAccessModifier
    ) -> DiagnosticGroup;

    auto CreateSymbolRedeclarationError(
        const ISymbol* const originalSymbol,
        const ISymbol* const redeclaredSymbol
    ) -> DiagnosticGroup;

    auto CreateStructFieldCausesCycleError(
        FieldVarSymbol* const fieldSymbol
    ) -> DiagnosticGroup;

    auto CreateUnableToDeduceTypeArgsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateUnableToDeduceTypeArgError(
        const SrcLocation& srcLocation,
        const TypeParamTypeSymbol* const typeParam
    ) -> DiagnosticGroup;

    auto CreateTooManyTypeArgsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateTypeArgDeductionConflict(
        const SrcLocation& srcLocation,
        const TypeParamTypeSymbol* const typeParam,
        const ITypeSymbol* const deducedArg,
        const ITypeSymbol* const conflictingDeducedArg
    ) -> DiagnosticGroup;

    auto CreateUndeclaredSymbolRefError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateAmbiguousSymbolRefError(
        const SrcLocation& srcLocation,
        const std::vector<ISymbol*>& candidateSymbols
    ) -> DiagnosticGroup;

    auto CreateScopeAccessOfNonBodyScopedSymbolError(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol
    ) -> DiagnosticGroup;

    auto CreateInaccessibleSymbolError(
        const SrcLocation& srcLocation,
        ISymbol* const symbol
    ) -> DiagnosticGroup;

    auto CreateIncorrectSymbolCategoryError(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol,
        const SymbolCategory expectedCategory
    ) -> DiagnosticGroup;

    auto CreateMissingStructFieldsError(
        const SrcLocation& srcLocation,
        StructTypeSymbol* const structSymbol,
        const std::vector<FieldVarSymbol*>& missingFieldSymbols
    ) -> DiagnosticGroup;

    auto CreateStructHasNoFieldNamedError(
        StructTypeSymbol* const structSymbol,
        const Ident& fieldName
    ) -> DiagnosticGroup;

    auto CreateStructFieldInitializedMoreThanOnceError(
        const SrcLocation& srcLocation,
        const SrcLocation& previousSrcLocation
    ) -> DiagnosticGroup;

    auto CreateUndeclaredUnaryOpError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const type
    ) -> DiagnosticGroup;

    auto CreateUndeclaredBinaryOpError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const lhsType,
        ITypeSymbol* const rhsType
    ) -> DiagnosticGroup;

    auto CreateAmbiguousBinaryOpRefError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const lhsType,
        ITypeSymbol* const rhsType
    ) -> DiagnosticGroup;

    auto CreateExpectedFunctionError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const type
    ) -> DiagnosticGroup;

    auto CreateInherentImplOfForeignTypeError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateSelfReferenceInIncorrectContext(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateAmbiguousTraitImplError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;
}
