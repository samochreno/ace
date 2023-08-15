#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "TypeInfo.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"

namespace Ace
{
    auto CreateExpectedLValueExprError(
        const std::shared_ptr<const IExprBoundNode>& expr
    ) -> DiagnosticGroup;

    auto CreateUnableToConvertExprError(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const TypeInfo& targetTypeInfo
    ) -> DiagnosticGroup;

    auto CreateExpectedDerefableExprError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateExpectedPtrExprError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateUnexpectedArgCountError(
        const SrcLocation& srcLocation,
        FunctionSymbol* const functionSymbol,
        const size_t expectedArgCount,
        const size_t unexpectedArgCount
    ) -> DiagnosticGroup;

    auto CreateExpectedStrongPtrExprError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateReturningExprFromVoidFunctionError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateReturningUnsizedExprError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateMissingReturnExprError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateMismatchedSelfExprTypeError(
        const SrcLocation& srcLocation,
        const SelfParamVarSymbol* const selfParamSymbol
    ) -> DiagnosticGroup;
}
