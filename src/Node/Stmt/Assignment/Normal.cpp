#include "Node/Stmt/Assignment/Normal.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/Assignment/Normal.hpp"

namespace Ace::Node::Stmt::Assignment
{
    Normal::Normal(
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<const Node::Expr::IBase>& t_lhsExpr,
        const std::shared_ptr<const Node::Expr::IBase>& t_rhsExpr
    ) : m_Scope{ t_scope },
        m_LHSExpr{ t_lhsExpr },
        m_RHSExpr{ t_rhsExpr }
    {
    }

    auto Normal::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Normal::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto Normal::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::Assignment::Normal>
    {
        return std::make_shared<const Node::Stmt::Assignment::Normal>(
            m_Scope,
            m_LHSExpr->CloneInScopeExpr(t_scope),
            m_RHSExpr->CloneInScopeExpr(t_scope)
        );
    }

    auto Normal::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto Normal::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Assignment::Normal>>
    {
        ACE_TRY(boundLHSExpr, m_LHSExpr->CreateBoundExpr());
        ACE_TRY(boundRHSExpr, m_RHSExpr->CreateBoundExpr());
        return std::make_shared<const BoundNode::Stmt::Assignment::Normal>(
            boundLHSExpr,
            boundRHSExpr
        );
    }

    auto Normal::CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return CreateBound();
    }
}
