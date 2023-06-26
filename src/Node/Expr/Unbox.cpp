#include "Node/Expr/Unbox.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "BoundNode/Expr/Box.hpp"

namespace Ace::Node::Expr
{
    Unbox::Unbox(
        const std::shared_ptr<const Node::Expr::IBase>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto Unbox::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto Unbox::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto Unbox::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::Unbox>
    {
        return std::make_shared<const Node::Expr::Unbox>(
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto Unbox::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto Unbox::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::Unbox>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const BoundNode::Expr::Unbox>(boundExpr);
    }

    auto Unbox::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }
}
