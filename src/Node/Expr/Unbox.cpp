#include "Node/Expr/Unbox.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbol/Type/Base.hpp"
#include "BoundNode/Expr/Box.hpp"

namespace Ace::Node::Expr
{
    auto Unbox::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto Unbox::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::Unbox>
    {
        return std::make_shared<const Node::Expr::Unbox>(
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto Unbox::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::Unbox>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const BoundNode::Expr::Unbox>(boundExpr);
    }
}
