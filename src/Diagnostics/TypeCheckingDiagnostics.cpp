#include "Diagnostics/TypeCheckingDiagnostics.hpp"

#include <memory>

#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    auto CreateExpectedLValueExprError(
        const std::shared_ptr<const IExprBoundNode>& expr
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            expr->GetSrcLocation(),
            "expected an assignable expression"
        );

        return group;
    }

    auto CreateUnableToConvertExprError(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const TypeInfo& targetTypeInfo
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        const std::string message =
            "unable to convert `" + expr->GetTypeInfo().Symbol->CreateSignature() +
            "` to `" + targetTypeInfo.Symbol->CreateSignature() + "`";

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            expr->GetSrcLocation(),
            message
        );

        return group;
    }

    auto CreateExpectedDerefableExprError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "expected a dereferencable expression"
        );

        return group;
    }

    auto CreateUnexpectedArgCountError(
        const SrcLocation& srcLocation,
        FunctionSymbol* const functionSymbol,
        const size_t expectedArgCount,
        const size_t unexpectedArgCount
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        const std::string message =
            "expected " + std::to_string(expectedArgCount) +
            " argument" + (expectedArgCount == 1 ? "" : "s") +
            ", got " + std::to_string(unexpectedArgCount);

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            functionSymbol->GetName().SrcLocation,
            "function declaration"
        );

        return group;
    }

    auto CreateExpectedStrongPtrExprError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "expected a strong pointer"
        );

        return group;
    }

    auto CreateReturningExprFromVoidFunctionError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        auto* const compilation = srcLocation.Buffer->GetCompilation();
        auto* const voidType = compilation->GetNatives()->Void.GetSymbol();
        
        const std::string message =
            "returning an expression from a function of type `" +
            voidType->CreateSignature() + "`";

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        return group;
    }

    auto CreateReturningUnsizedExprError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "returning an unsized expression"
        );

        return group;
    }

    auto CreateMissingReturnExprError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "missing return expression"
        );

        return group;
    }
}
