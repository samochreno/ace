#include "BoundNodes/Exprs/UserUnaryExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Assert.hpp"

namespace Ace
{
    UserUnaryExprBoundNode::UserUnaryExprBoundNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<const IExprBoundNode>& t_expr,
        FunctionSymbol* const t_opSymbol
    ) : m_SourceLocation{ t_sourceLocation },
        m_Expr{ t_expr },
        m_OpSymbol{ t_opSymbol }
    {
    }

    auto UserUnaryExprBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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

        return CreateChanged(std::make_shared<const UserUnaryExprBoundNode>(
            GetSourceLocation(),
            mchCheckedExpr.Value,
            m_OpSymbol
        ));
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

        return CreateChanged(std::make_shared<const StaticFunctionCallExprBoundNode>(
            GetSourceLocation(),
            GetScope(),
            m_OpSymbol,
            std::vector{ mchLoweredExpr.Value }
        )->GetOrCreateLowered({}).Value);
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
        return { m_OpSymbol->GetType(), ValueKind::R };
    }
}
