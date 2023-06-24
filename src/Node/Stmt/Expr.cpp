#include "Node/Stmt/Expr.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Stmt/Expr.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Stmt
{
    auto Expr::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto Expr::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::Expr>
    {
        return std::make_shared<const Node::Stmt::Expr>(
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto Expr::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Expr>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const BoundNode::Stmt::Expr>(boundExpr);
    }
}
