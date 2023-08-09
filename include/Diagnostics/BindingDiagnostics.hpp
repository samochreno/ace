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

    auto CreateSymbolRedefinitionError(
        const ISymbol* const originalSymbol,
        const ISymbol* const redefinedSymbol
    ) -> DiagnosticGroup;

    auto CreateUnsizedSymbolTypeError(
        ISymbol* const symbol
    ) -> DiagnosticGroup;

    auto CreateStructVarCausesCycleError(
        InstanceVarSymbol* const varSymbol
    ) -> DiagnosticGroup;

    auto CreateUnableToDeduceTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateUnableToDeduceTemplateArgError(
        const SrcLocation& srcLocation,
        const NormalTemplateParamTypeSymbol* const templateParam
    ) -> DiagnosticGroup;

    auto CreateTooManyTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateTemplateArgDeductionConflict(
        const SrcLocation& srcLocation,
        const NormalTemplateParamTypeSymbol* const templateParam,
        const ITypeSymbol* const deducedArg,
        const ITypeSymbol* const conflictingDeducedArg
    ) -> DiagnosticGroup;

    auto CreateUndefinedSymbolRefError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateAmbiguousSymbolRefError(
        const SrcLocation& srcLocation,
        const std::vector<ISymbol*>& candidateSymbols
    ) -> DiagnosticGroup;

    auto CreateScopeAccessOfNonSelfScopedSymbolError(
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

    auto CreateMissingStructVarsError(
        const SrcLocation& srcLocation,
        StructTypeSymbol* const structSymbol,
        const std::vector<InstanceVarSymbol*>& missingVarSymbols
    ) -> DiagnosticGroup;

    auto CreateStructHasNoVarNamedError(
        StructTypeSymbol* const structSymbol,
        const Ident& fieldName
    ) -> DiagnosticGroup;

    auto CreateStructVarInitializedMoreThanOnceError(
        const SrcLocation& srcLocation,
        const SrcLocation& previousSrcLocation
    ) -> DiagnosticGroup;

    auto CreateUndefinedUnaryOpError(
        const Op& op,
        ITypeSymbol* const type
    ) -> DiagnosticGroup;

    auto CreateUndefinedBinaryOpError(
        const Op& op,
        ITypeSymbol* const lhsType,
        ITypeSymbol* const rhsType
    ) -> DiagnosticGroup;

    auto CreateAmbiguousBinaryOpRefError(
        const Op& op,
        ITypeSymbol* const lhsType,
        ITypeSymbol* const rhsType
    ) -> DiagnosticGroup;

    auto CreateExpectedFunctionError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const type
    ) -> DiagnosticGroup;
}
