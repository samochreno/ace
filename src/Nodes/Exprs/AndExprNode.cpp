#include "Nodes/Exprs/AndExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/AndExprBoundNode.hpp"

namespace Ace
{
    AndExprNode::AndExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& lhsExpr,
        const std::shared_ptr<const IExprNode>& rhsExpr
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto AndExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AndExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto AndExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto AndExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const AndExprNode>
    {
        return std::make_shared<const AndExprNode>(
            m_SrcLocation,
            m_LHSExpr->CloneInScopeExpr(scope),
            m_RHSExpr->CloneInScopeExpr(scope)
        );
    }

    auto AndExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto AndExprNode::CreateBound() const -> Diagnosed<std::shared_ptr<const AndExprBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto boundLHSExpr =
            diagnostics.Collect(m_LHSExpr->CreateBoundExpr());
        const auto boundRHSExpr =
            diagnostics.Collect(m_RHSExpr->CreateBoundExpr());

        return Diagnosed
        {
            std::make_shared<const AndExprBoundNode>(
                GetSrcLocation(),
                boundLHSExpr,
                boundRHSExpr
            ),
            std::move(diagnostics),
        };
    }

    auto AndExprNode::CreateBoundExpr() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
