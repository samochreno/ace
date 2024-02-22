#include "Diagnostics/TypeCheckingDiagnostics.hpp"

#include <memory>

#include "Diagnostic.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    auto CreateExpectedLValueExprError(
        const std::shared_ptr<const IExprSema>& expr
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            expr->GetSrcLocation(),
            "expected an assignable expression"
        );

        return group;
    }

    auto CreateExpectedSizedExprError(
        const std::shared_ptr<const IExprSema>& expr
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            expr->GetSrcLocation(),
            "expected a sized expression"
        );

        return group;
    }

    auto CreateUnableToConvertExprError(
        const std::shared_ptr<const IExprSema>& expr,
        const TypeInfo& targetTypeInfo
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unable to convert `" +
            expr->GetTypeInfo().Symbol->CreateDisplayName() + "` to `" +
            targetTypeInfo.Symbol->CreateDisplayName() + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            expr->GetSrcLocation(),
            message
        );

        return group;
    }

    auto CreateExpectedDerefableExprError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "expected a dereferencable expression"
        );

        return group;
    }

    auto CreateExpectedPtrError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "expected a pointer"
        );

        return group;
    }

    auto CreateExpectedStrongPtrError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "expected a strong pointer"
        );

        return group;
    }

    auto CreateExpectedNonDynStrongPtrError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "expected a non-trait strong pointer"
        );

        return group;
    }

    auto CreateUnexpectedArgCountError(
        const SrcLocation& srcLocation,
        ICallableSymbol* const callableSymbol,
        const size_t expectedArgCount,
        const size_t unexpectedArgCount
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "expected " + std::to_string(expectedArgCount) +
            " argument" + (expectedArgCount == 1 ? "" : "s") +
            ", got " + std::to_string(unexpectedArgCount);

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            callableSymbol->GetName().SrcLocation,
            "function declaration"
        );

        return group;
    }

    auto CreateExprRetFromVoidFunctionError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        auto* const compilation = srcLocation.Buffer->GetCompilation();
        auto* const voidType = compilation->GetVoidTypeSymbol();
        
        const std::string message =
            "returning an expression from a function of type `" +
            voidType->CreateDisplayName() + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        return group;
    }

    auto CreateUnsizedRetExprError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "returning an unsized expression"
        );

        return group;
    }

    auto CreateMissingRetExprError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "missing return expression"
        );

        return group;
    }

    auto CreateMismatchedSelfExprTypeError(
        const SrcLocation& srcLocation,
        const SelfParamVarSymbol* const selfParamSymbol
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "mismatched self argument type"
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            selfParamSymbol->GetName().SrcLocation,
            "self parameter declaration"
        );

        return group;
    }

    auto CreateFunctionNotDynDispatchableError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "function is not dynamically dispatchable"
        );

        return group;
    }
}
