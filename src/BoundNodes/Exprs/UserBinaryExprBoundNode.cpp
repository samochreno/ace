#include "BoundNodes/Exprs/UserBinaryExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    UserBinaryExprBoundNode::UserBinaryExprBoundNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<const IExprBoundNode>& t_lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& t_rhsExpr,
        FunctionSymbol* const t_opSymbol
    ) : m_SourceLocation{ t_sourceLocation },
        m_LHSExpr{ t_lhsExpr },
        m_RHSExpr{ t_rhsExpr },
        m_OpSymbol{ t_opSymbol }
    {
    }

    auto UserBinaryExprBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto UserBinaryExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto UserBinaryExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto UserBinaryExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const UserBinaryExprBoundNode>>>
    {
        const auto argTypeInfos = m_OpSymbol->CollectArgTypeInfos();

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

        return CreateChanged(std::make_shared<const UserBinaryExprBoundNode>(
            GetSourceLocation(),
            mchConvertedAndCheckedLHSExpr.Value,
            mchConvertedAndCheckedRHSExpr.Value,
            m_OpSymbol
        ));
    }

    auto UserBinaryExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto UserBinaryExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const StaticFunctionCallExprBoundNode>>
    {
        const auto mchLoweredLHSExpr = m_LHSExpr->GetOrCreateLoweredExpr({});
        const auto mchLoweredRHSExpr = m_RHSExpr->GetOrCreateLoweredExpr({});

        return CreateChanged(std::make_shared<const StaticFunctionCallExprBoundNode>(
            GetSourceLocation(),
            GetScope(),
            m_OpSymbol,
            std::vector
            {
                mchLoweredLHSExpr.Value,
                mchLoweredRHSExpr.Value
            }
        )->GetOrCreateLowered({}).Value);
    }

    auto UserBinaryExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto UserBinaryExprBoundNode::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UserBinaryExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_OpSymbol->GetType(), ValueKind::R };
    }
}
