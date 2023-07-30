#include "BoundNodes/Exprs/UserBinaryExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    UserBinaryExprBoundNode::UserBinaryExprBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& rhsExpr,
        FunctionSymbol* const opSymbol
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr },
        m_OpSymbol{ opSymbol }
    {
    }

    auto UserBinaryExprBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto UserBinaryExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UserBinaryExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto UserBinaryExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto UserBinaryExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
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
            DiagnosticBag{},
            GetSrcLocation(),
            mchConvertedAndCheckedLHSExpr.Value,
            mchConvertedAndCheckedRHSExpr.Value,
            m_OpSymbol
        ));
    }

    auto UserBinaryExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto UserBinaryExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const StaticFunctionCallExprBoundNode>>
    {
        const auto mchLoweredLHSExpr = m_LHSExpr->GetOrCreateLoweredExpr({});
        const auto mchLoweredRHSExpr = m_RHSExpr->GetOrCreateLoweredExpr({});

        return CreateChanged(std::make_shared<const StaticFunctionCallExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
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
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto UserBinaryExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UserBinaryExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_OpSymbol->GetType(), ValueKind::R };
    }
}
