#include "Node/Expression/Literal.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Error.hpp"
#include "BoundNode/Expression/Literal.hpp"

namespace Ace::Node::Expression
{
    auto Literal::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto Literal::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::Literal>
    {
        return std::make_shared<const Node::Expression::Literal>(
            t_scope,
            m_Kind,
            m_String
        );
    }

    auto Literal::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::Literal>>
    {
        return std::make_shared<const BoundNode::Expression::Literal>(
            m_Scope, 
            m_Kind,
            m_String
        );
    }
}
