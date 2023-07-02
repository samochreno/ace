#include "BoundNodes/Exprs/UserUnaryExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Asserts.hpp"

namespace Ace
{
    UserUnaryExprBoundNode::UserUnaryExprBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_expr,
        FunctionSymbol* const t_operatorSymbol
    ) : m_Expr{ t_expr },
        m_OperatorSymbol{ t_operatorSymbol }
    {
    }

    auto UserUnaryExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UserUnaryExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto UserUnaryExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const UserUnaryExprBoundNode>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const UserUnaryExprBoundNode>(
            mchCheckedExpr.Value,
            m_OperatorSymbol
        );
        return CreateChanged(returnValue);
    }

    auto UserUnaryExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto UserUnaryExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const StaticFunctionCallExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        const auto returnValue = std::make_shared<const StaticFunctionCallExprBoundNode>(
            GetScope(),
            m_OperatorSymbol,
            std::vector{ mchLoweredExpr.Value }
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto UserUnaryExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto UserUnaryExprBoundNode::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UserUnaryExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_OperatorSymbol->GetType(), ValueKind::R };
    }
}