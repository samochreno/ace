#pragma once

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/All.hpp"

namespace Ace
{
    auto CreateUnsizedTypeArgError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateUnimplementedTraitFunctionError(
        TraitImplSymbol* const impl,
        ISymbol* const function
    ) -> DiagnosticGroup;

    auto CreateMismatchedTraitImplTypeError(
        ISymbol* const symbol,
        const SrcLocation& functionTypeSrcLocation,
        const SrcLocation& prototypeTypeSrcLocation
    ) -> DiagnosticGroup;

    auto CreateMismatchedTraitImplFunctionParamCountError(
        PrototypeSymbol* const prototype,
        FunctionSymbol* const function
    ) -> DiagnosticGroup;

    auto CreateMismatchedTraitImplFunctionTypeParamCountError(
        PrototypeSymbol* const prototype,
        FunctionSymbol* const function
    ) -> DiagnosticGroup;

    auto CreateInherentFunctionRedeclarationError(
        FunctionSymbol* const originalFunction,
        FunctionSymbol* const redeclaredFunction
    ) -> DiagnosticGroup;

    auto CreateFunctionIsNotMemberOfTraitError(
        FunctionSymbol* const function,
        TraitTypeSymbol* const traitType
    ) -> DiagnosticGroup;

    auto CreateImplHasStricterConstraintsThanPrototypeError(
        PrototypeSymbol* const prototype,
        FunctionSymbol* const function
    ) -> DiagnosticGroup;

    auto CreateOverlappingInherentImplSymbolError(
        ISymbol* const overlappingSymbol,
        ISymbol* const symbol
    ) -> DiagnosticGroup;

    auto CreateNotAllControlPathsRetError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateOrphanInherentImplError(
        InherentImplSymbol* const impl
    ) -> DiagnosticGroup;

    auto CreateOrphanTraitImplError(
        TraitImplSymbol* const impl
    ) -> DiagnosticGroup;

    auto CreateConcreteConstraintError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;
}
