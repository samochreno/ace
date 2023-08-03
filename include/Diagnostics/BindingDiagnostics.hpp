#pragma once

#include <memory>
#include <string>

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
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateSymbolRedefinitionError(
        const ISymbol* const originalSymbol,
        const ISymbol* const redefinedSymbol
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnableToDeduceTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnableToDeduceTemplateArgError(
        const SrcLocation& srcLocation,
        const NormalTemplateParamTypeSymbol* const templateParam
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateTooManyTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateTemplateArgDeductionConflict(
        const SrcLocation& srcLocation,
        const NormalTemplateParamTypeSymbol* const templateParam,
        const ITypeSymbol* const deducedArg,
        const ITypeSymbol* const conflictingDeducedArg
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUndefinedSymbolRefError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateAmbiguousSymbolRefError(
        const SrcLocation& srcLocation,
        const std::vector<ISymbol*>& candidateSymbols
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateScopeAccessOfNonSelfScopedSymbolError(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateInaccessibleSymbolError(
        const SrcLocation& srcLocation,
        ISymbol* const symbol
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateIncorrectSymbolCategoryError(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol,
        const SymbolCategory expectedCategory
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateMissingStructVarsError(
        const SrcLocation& srcLocation,
        StructTypeSymbol* const structSymbol,
        const std::vector<InstanceVarSymbol*>& missingVarSymbols
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateStructHasNoVarNamedError(
        StructTypeSymbol* const structSymbol,
        const Ident& fieldName
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateStructVarInitializedMoreThanOnceError(
        const SrcLocation& srcLocation,
        const SrcLocation& previousSrcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUndefinedUnaryOpRefError(
        const Op& op,
        ITypeSymbol* const type
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUndefinedBinaryOpRefError(
        const Op& op,
        ITypeSymbol* const lhsType,
        ITypeSymbol* const rhsType
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateAmbiguousBinaryOpRefError(
        const Op& op,
        ITypeSymbol* const lhsType,
        ITypeSymbol* const rhsType
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateExpectedFunctionError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const type
    ) -> std::shared_ptr<const DiagnosticGroup>;
}
