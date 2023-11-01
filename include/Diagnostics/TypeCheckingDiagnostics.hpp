#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "TypeInfo.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "Symbols/CallableSymbol.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"

namespace Ace
{
    auto CreateExpectedLValueExprError(
        const std::shared_ptr<const IExprSema>& expr
    ) -> DiagnosticGroup;

    auto CreateExpectedSizedExprError(
        const std::shared_ptr<const IExprSema>& expr
    ) -> DiagnosticGroup;

    auto CreateExpectedSizedExprError(
        const std::shared_ptr<const IExprSema>& expr
    ) -> DiagnosticGroup;

    auto CreateUnableToConvertExprError(
        const std::shared_ptr<const IExprSema>& expr,
        const TypeInfo& targetTypeInfo
    ) -> DiagnosticGroup;

    auto CreateExpectedDerefableExprError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateExpectedPtrError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateExpectedStrongPtrError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateExpectedNonDynStrongPtrError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

   auto CreateUnexpectedArgCountError(
        const SrcLocation& srcLocation,
        ICallableSymbol* const callableSymbol,
        const size_t expectedArgCount,
        const size_t unexpectedArgCount
    ) -> DiagnosticGroup;

    auto CreateExprRetFromVoidFunctionError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateUnsizedRetExprError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateMissingRetExprError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateMismatchedSelfExprTypeError(
        const SrcLocation& srcLocation,
        const SelfParamVarSymbol* const selfParamSymbol
    ) -> DiagnosticGroup;

    auto CreateFunctionNotDynDispatchableError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;
}
