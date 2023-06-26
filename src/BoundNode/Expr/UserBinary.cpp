#include "BoundNode/Expr/UserBinary.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expr/FunctionCall/Static.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Expr
{
    UserBinary::UserBinary(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_lhsExpr,
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_rhsExpr,
        FunctionSymbol* const t_operatorSymbol
    ) : m_LHSExpr{ t_lhsExpr },
        m_RHSExpr{ t_rhsExpr },
        m_OperatorSymbol{ t_operatorSymbol }
    {
    }

    auto UserBinary::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto UserBinary::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto UserBinary::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::UserBinary>>>
    {
        const auto argTypeInfos =
            m_OperatorSymbol->CollectArgTypeInfos();

        ACE_TRY(mchConvertedAndCheckedLHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_LHSExpr,
            argTypeInfos.at(0)
        ));

        ACE_TRY(mchConvertedAndCheckedRHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_RHSExpr,
            argTypeInfos.at(1)
        ));

        if (
            !mchConvertedAndCheckedLHSExpr.IsChanged &&
            !mchConvertedAndCheckedRHSExpr.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::UserBinary>(
            mchConvertedAndCheckedLHSExpr.Value,
            mchConvertedAndCheckedRHSExpr.Value,
            m_OperatorSymbol
        );
        return CreateChanged(returnValue);
    }

    auto UserBinary::GetOrCreateTypeCheckedExpr(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto UserBinary::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::FunctionCall::Static>>
    {
        const auto mchLoweredLHSExpr =
            m_LHSExpr->GetOrCreateLoweredExpr({});

        const auto mchLoweredRHSExpr =
            m_RHSExpr->GetOrCreateLoweredExpr({});

        const auto returnValue = std::make_shared<const BoundNode::Expr::FunctionCall::Static>(
            GetScope(),
            m_OperatorSymbol,
            std::vector
            {
                mchLoweredLHSExpr.Value,
                mchLoweredRHSExpr.Value
            }
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto UserBinary::GetOrCreateLoweredExpr(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto UserBinary::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UserBinary::GetTypeInfo() const -> TypeInfo
    {
        return { m_OperatorSymbol->GetType(), ValueKind::R };
    }
}
