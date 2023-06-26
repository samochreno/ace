#include "Node/Expr/Expr.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expr/Expr.hpp"

namespace Ace::Node::Expr
{
    Expr::Expr(
        const std::shared_ptr<const Node::Expr::IBase>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto Expr::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto Expr::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto Expr::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::Expr>
    {
        return std::make_shared<const Node::Expr::Expr>(
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto Expr::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto Expr::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::Expr>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const BoundNode::Expr::Expr>(boundExpr);
    }

    auto Expr::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }
}
