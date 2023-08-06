#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "TypeInfo.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    auto CreateExpectedLValueExprError(
        const std::shared_ptr<const IExprBoundNode>& expr
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnableToConvertExprError(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const TypeInfo& targetTypeInfo
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateExpectedDerefableExprError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnexpectedArgCountError(
        const SrcLocation& srcLocation,
        FunctionSymbol* const functionSymbol,
        const size_t expectedArgCount,
        const size_t unexpectedArgCount
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateExpectedStrongPtrExprError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateReturningExprFromVoidFunctionError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateReturningUnsizedExprError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateMissingReturnExprError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;
}
