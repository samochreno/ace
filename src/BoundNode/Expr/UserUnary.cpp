#include "BoundNode/Expr/UserUnary.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expr/FunctionCall/Static.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Asserts.hpp"

namespace Ace::BoundNode::Expr
{
    UserUnary::UserUnary(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr,
        FunctionSymbol* const t_operatorSymbol
    ) : m_Expr{ t_expr },
        m_OperatorSymbol{ t_operatorSymbol }
    {
    }

    auto UserUnary::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UserUnary::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto UserUnary::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::UserUnary>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::UserUnary>(
            mchCheckedExpr.Value,
            m_OperatorSymbol
        );
        return CreateChanged(returnValue);
    }

    auto UserUnary::GetOrCreateTypeCheckedExpr(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto UserUnary::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::FunctionCall::Static>>
    {
        const auto mchLoweredExpr =
            m_Expr->GetOrCreateLoweredExpr({});

        const auto returnValue = std::make_shared<const BoundNode::Expr::FunctionCall::Static>(
            GetScope(),
            m_OperatorSymbol,
            std::vector{ mchLoweredExpr.Value }
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto UserUnary::GetOrCreateLoweredExpr(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto UserUnary::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UserUnary::GetTypeInfo() const -> TypeInfo
    {
        return { m_OperatorSymbol->GetType(), ValueKind::R };
    }
}
