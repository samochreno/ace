#include "Node/Expr/Box.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbol/Type/Base.hpp"
#include "BoundNode/Expr/Box.hpp"

namespace Ace::Node::Expr
{
    auto Box::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto Box::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::Box>
    {
        return std::make_shared<const Node::Expr::Box>(
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto Box::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::Box>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const BoundNode::Expr::Box>(boundExpr);
    }
}
