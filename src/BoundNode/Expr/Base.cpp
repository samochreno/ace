#include "BoundNode/Expr/Base.hpp"

#include <memory>
#include <vector>
#include <optional>
#include <algorithm>
#include <iterator>

#include "BoundNode/Expr/FunctionCall/Static.hpp"
#include "BoundNode/Expr//Reference.hpp"
#include "BoundNode/Expr//Dereference.hpp"
#include "TypeInfo.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Expr
{
    auto CreateConvertedVector(
        const std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>& t_exprs,
        const TypeInfo& t_targetTypeInfo,
        ConversionFunction t_func
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>>>
    {
        bool isChanged = false;
        std::vector<std::shared_ptr<const BoundNode::Expr::IBase>> convertedExprs{};
        convertedExprs.reserve(t_exprs.size());

        ACE_TRY_ASSERT(std::find_if_not(begin(t_exprs), end(t_exprs),
        [&](const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr)
        {
            auto expMchConvertedExpr = t_func(t_expr, t_targetTypeInfo);
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

        }) == end(t_exprs));

        if (!isChanged)
        {
            return CreateUnchanged(t_exprs);
        }

        return CreateChanged(convertedExprs);
    }

    auto CreateConvertedVector(
        const std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>& t_exprs,
        const std::vector<TypeInfo>& t_targetTypeInfos,
        ConversionFunction t_func
    ) -> Expected<MaybeChanged<std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>>>
    {
        bool isChanged = false;
        std::vector<std::shared_ptr<const BoundNode::Expr::IBase>> convertedExprs{};
        convertedExprs.reserve(t_exprs.size());

        for (size_t i = 0; i < t_exprs.size(); i++)
        {
            ACE_TRY(mchConvertedExpr, t_func(
                t_exprs.at(i),
                t_targetTypeInfos.at(i)
            ));

            if (mchConvertedExpr.IsChanged)
            {
                isChanged = true;
            }

            convertedExprs.push_back(mchConvertedExpr.Value);
        }

        if (!isChanged)
        {
            return CreateUnchanged(t_exprs);
        }

        return CreateChanged(convertedExprs);
    }

    auto CreateConverted(
        std::shared_ptr<const BoundNode::Expr::IBase> t_expr,
        TypeInfo t_targetTypeInfo,
        ConversionOperatorGetterFunction t_func
    ) -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>
    {
        if (t_targetTypeInfo.ValueKind == ValueKind::L)
        {
            ACE_TRY_ASSERT(
                t_expr->GetTypeInfo().ValueKind != ValueKind::R
            );
        }

        if (
            t_expr->GetTypeInfo().Symbol->GetUnaliased() ==
            t_targetTypeInfo.Symbol->GetUnaliased()
            )
        {
            return CreateUnchanged(t_expr);
        }

        const bool isReference = t_expr->GetTypeInfo().Symbol->IsReference();
        const bool isTargetReference = t_targetTypeInfo.Symbol->IsReference();

        if (isReference)
        {
            if (!isTargetReference)
            {
                t_expr = std::make_shared<const BoundNode::Expr::Dereference>(
                    t_expr
                );
            }
        }
        else
        {
            if (isTargetReference)
            {
                t_expr = std::make_shared<const BoundNode::Expr::Reference>(
                    t_expr
                );
            }
        }

        if (
            t_expr->GetTypeInfo().Symbol->GetUnaliased() ==
            t_targetTypeInfo.Symbol->GetUnaliased()
            )
        {
            return CreateChanged(t_expr);
        }

        ACE_TRY(operatorSymbol, t_func(
            t_expr->GetScope(), 
            t_expr->GetTypeInfo().Symbol,
            t_targetTypeInfo.Symbol
        ));

        t_expr = std::make_shared<const BoundNode::Expr::FunctionCall::Static>(
            t_expr->GetScope(),
            operatorSymbol,
            std::vector{ t_expr }
        );

        return CreateChanged(t_expr);
    }

    auto CreateImplicitlyConvertedAndTypeChecked(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr,
        const TypeInfo& t_targetTypeInfo
    ) -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>
    {
        ACE_TRY(mchConverted, CreateImplicitlyConverted(
            t_expr,
            t_targetTypeInfo
        ));

        ACE_TRY(mchChecked, mchConverted.Value->GetOrCreateTypeCheckedExpr({}));

        if (
            !mchConverted.IsChanged &&
            !mchChecked.IsChanged
            )
        {
            return CreateUnchanged(t_expr);
        }

        return CreateChanged(mchChecked.Value);
    }

    auto CreateImplicitlyConvertedAndTypeCheckedOptional(
        const std::optional<std::shared_ptr<const BoundNode::Expr::IBase>>& t_optExpr,
        const TypeInfo& t_targetTypeInfo
    ) -> Expected<MaybeChanged<std::optional<std::shared_ptr<const BoundNode::Expr::IBase>>>>
    {
        if (!t_optExpr.has_value())
        {
            return CreateUnchanged(
                std::optional<std::shared_ptr<const BoundNode::Expr::IBase>>{}
            );
        }

        ACE_TRY(mchConvertedAndChecked, CreateImplicitlyConvertedAndTypeChecked(
            t_optExpr.value(),
            t_targetTypeInfo
        ));

        return MaybeChanged<std::optional<std::shared_ptr<const BoundNode::Expr::IBase>>>
        {
            mchConvertedAndChecked.IsChanged,
            mchConvertedAndChecked.Value
        };
    }
}
