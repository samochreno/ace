#include "BoundNodes/Exprs/UserBinaryExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Cacheable.hpp"

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

    auto UserBinaryExprBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const UserBinaryExprBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const UserBinaryExprBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_LHSExpr,
            m_RHSExpr,
            m_OpSymbol
        );
    }

    auto UserBinaryExprBoundNode::CloneWithDiagnosticsExpr(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto UserBinaryExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const UserBinaryExprBoundNode>>>
    {
        const auto argTypeInfos = m_OpSymbol->CollectArgTypeInfos();

        ACE_TRY(cchConvertedAndCheckedLHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_LHSExpr,
            argTypeInfos.at(0)
        ));

        ACE_TRY(cchConvertedAndCheckedRHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_RHSExpr,
            argTypeInfos.at(1)
        ));

        if (
            !cchConvertedAndCheckedLHSExpr.IsChanged &&
            !cchConvertedAndCheckedRHSExpr.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const UserBinaryExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchConvertedAndCheckedLHSExpr.Value,
            cchConvertedAndCheckedRHSExpr.Value,
            m_OpSymbol
        ));
    }

    auto UserBinaryExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto UserBinaryExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const StaticFunctionCallExprBoundNode>>
    {
        const auto cchLoweredLHSExpr = m_LHSExpr->GetOrCreateLoweredExpr({});
        const auto cchLoweredRHSExpr = m_RHSExpr->GetOrCreateLoweredExpr({});

        return CreateChanged(std::make_shared<const StaticFunctionCallExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            m_OpSymbol,
            std::vector
            {
                cchLoweredLHSExpr.Value,
                cchLoweredRHSExpr.Value
            }
        )->GetOrCreateLowered({}).Value);
    }

    auto UserBinaryExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IExprBoundNode>>
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
