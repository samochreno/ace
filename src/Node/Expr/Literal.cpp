#include "Node/Expr/Literal.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expr/Literal.hpp"

namespace Ace::Node::Expr
{
    auto Literal::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto Literal::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::Literal>
    {
        return std::make_shared<const Node::Expr::Literal>(
            t_scope,
            m_Kind,
            m_String
        );
    }

    auto Literal::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::Literal>>
    {
        return std::make_shared<const BoundNode::Expr::Literal>(
            m_Scope, 
            m_Kind,
            m_String
        );
    }
}
