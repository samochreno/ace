#include "Node/Expr/Or.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expr/Or.hpp"

namespace Ace::Node::Expr
{
    Or::Or(
        const std::shared_ptr<const Node::Expr::IBase>& t_lhsExpr,
        const std::shared_ptr<const Node::Expr::IBase>& t_rhsExpr
    ) : m_LHSExpr{ t_lhsExpr },
        m_RHSExpr{ t_rhsExpr }
    {
    }

    auto Or::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto Or::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto Or::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::Or>
    {
        return std::make_shared<const Node::Expr::Or>(
            m_LHSExpr->CloneInScopeExpr(t_scope),
            m_RHSExpr->CloneInScopeExpr(t_scope)
        );
    }

    auto Or::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto Or::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::Or>>
    {
        ACE_TRY(boundLHSExpr, m_LHSExpr->CreateBoundExpr());
        ACE_TRY(boundRHSExpr, m_RHSExpr->CreateBoundExpr());
        return std::make_shared<const BoundNode::Expr::Or>(
            boundLHSExpr,
            boundRHSExpr
        );
    }

    auto Or::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }
}
