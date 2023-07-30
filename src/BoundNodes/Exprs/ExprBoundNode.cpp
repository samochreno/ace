#include "BoundNodes/Exprs/ExprBoundNode.hpp"

#include <memory>
#include <vector>
#include <optional>
#include <algorithm>
#include <iterator>

#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "BoundNodes/Exprs/RefExprBoundNode.hpp"
#include "BoundNodes/Exprs/DerefExprBoundNode.hpp"
#include "TypeInfo.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    auto CreateConvertedVector(
        const std::vector<std::shared_ptr<const IExprBoundNode>>& exprs,
        const TypeInfo& targetTypeInfo,
        ConversionFunction func
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const IExprBoundNode>>>>
    {
        bool isChanged = false;
        std::vector<std::shared_ptr<const IExprBoundNode>> convertedExprs{};
        convertedExprs.reserve(exprs.size());

        ACE_TRY_ASSERT(std::find_if_not(begin(exprs), end(exprs),
        [&](const std::shared_ptr<const IExprBoundNode>& expr)
        {
            auto expMchConvertedExpr = func(expr, targetTypeInfo);
            if (!expMchConvertedExpr)
            {
                return false;
            }

            if (expMchConvertedExpr.Unwrap().IsChanged)
            {
                isChanged = true;
            }

            convertedExprs.push_back(expMchConvertedExpr.Unwrap().Value);
            return true;

        }) == end(exprs));

        if (!isChanged)
        {
            return CreateUnchanged(exprs);
        }

        return CreateChanged(convertedExprs);
    }

    auto CreateConvertedVector(
        const std::vector<std::shared_ptr<const IExprBoundNode>>& exprs,
        const std::vector<TypeInfo>& targetTypeInfos,
        ConversionFunction func
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const IExprBoundNode>>>>
    {
        bool isChanged = false;
        std::vector<std::shared_ptr<const IExprBoundNode>> convertedExprs{};
        convertedExprs.reserve(exprs.size());

        for (size_t i = 0; i < exprs.size(); i++)
        {
            ACE_TRY(mchConvertedExpr, func(
                exprs.at(i),
                targetTypeInfos.at(i)
            ));

            if (mchConvertedExpr.IsChanged)
            {
                isChanged = true;
            }

            convertedExprs.push_back(mchConvertedExpr.Value);
        }

        if (!isChanged)
        {
            return CreateUnchanged(exprs);
        }

        return CreateChanged(convertedExprs);
    }

    auto CreateConverted(
        std::shared_ptr<const IExprBoundNode> expr,
        TypeInfo targetTypeInfo,
        ConversionOpGetterFunction func
    ) -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        if (targetTypeInfo.ValueKind == ValueKind::L)
        {
            ACE_TRY_ASSERT(expr->GetTypeInfo().ValueKind != ValueKind::R);
        }

        if (
            expr->GetTypeInfo().Symbol->GetUnaliased() ==
            targetTypeInfo.Symbol->GetUnaliased()
            )
        {
            return CreateUnchanged(expr);
        }

        const bool isRef = expr->GetTypeInfo().Symbol->IsRef();
        const bool isTargetRef = targetTypeInfo.Symbol->IsRef();

        if (isRef)
        {
            if (!isTargetRef)
            {
                expr = std::make_shared<const DerefExprBoundNode>(
                    DiagnosticBag{},
                    expr->GetSrcLocation(),
                    expr
                );
            }
        }
        else
        {
            if (isTargetRef)
            {
                expr = std::make_shared<const RefExprBoundNode>(
                    DiagnosticBag{},
                    expr->GetSrcLocation(),
                    expr
                );
            }
        }

        if (
            expr->GetTypeInfo().Symbol->GetUnaliased() ==
            targetTypeInfo.Symbol->GetUnaliased()
            )
        {
            return CreateChanged(expr);
        }

        ACE_TRY(opSymbol, func(
            expr->GetSrcLocation(),
            expr->GetScope(), 
            expr->GetTypeInfo().Symbol,
            targetTypeInfo.Symbol
        ));

        expr = std::make_shared<const StaticFunctionCallExprBoundNode>(
            DiagnosticBag{},
            expr->GetSrcLocation(),
            expr->GetScope(),
            opSymbol,
            std::vector{ expr }
        );

        return CreateChanged(expr);
    }

    auto CreateImplicitlyConvertedAndTypeChecked(
        const std::shared_ptr<const IExprBoundNode>& expr,
        const TypeInfo& targetTypeInfo
    ) -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        ACE_TRY(mchConverted, CreateImplicitlyConverted(
            expr,
            targetTypeInfo
        ));

        ACE_TRY(mchChecked, mchConverted.Value->GetOrCreateTypeCheckedExpr({}));

        if (
            !mchConverted.IsChanged &&
            !mchChecked.IsChanged
            )
        {
            return CreateUnchanged(expr);
        }

        return CreateChanged(mchChecked.Value);
    }

    auto CreateImplicitlyConvertedAndTypeCheckedOptional(
        const std::optional<std::shared_ptr<const IExprBoundNode>>& optExpr,
        const TypeInfo& targetTypeInfo
    ) -> Expected<MaybeChanged<std::optional<std::shared_ptr<const IExprBoundNode>>>>
    {
        if (!optExpr.has_value())
        {
            return CreateUnchanged(
                std::optional<std::shared_ptr<const IExprBoundNode>>{}
            );
        }

        ACE_TRY(mchConvertedAndChecked, CreateImplicitlyConvertedAndTypeChecked(
            optExpr.value(),
            targetTypeInfo
        ));

        return MaybeChanged<std::optional<std::shared_ptr<const IExprBoundNode>>>
        {
            mchConvertedAndChecked.IsChanged,
            mchConvertedAndChecked.Value,
        };
    }
}
