#include "BoundNodes/Exprs/ExprExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    ExprExprBoundNode::ExprExprBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto ExprExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto ExprExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto ExprExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ExprExprBoundNode>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const ExprExprBoundNode>(
            mchCheckedExpr.Value
        );
        return CreateChanged(returnValue);
    }

    auto ExprExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto ExprExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const ExprExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const ExprExprBoundNode>(
            mchLoweredExpr.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto ExprExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto ExprExprBoundNode::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        return m_Expr->Emit(t_emitter);
    }

    auto ExprExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return m_Expr->GetTypeInfo();
    }
}
