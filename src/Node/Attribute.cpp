#include "Node/Attribute.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Attribute.hpp"

namespace Ace::Node
{
    auto Attribute::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_StructConstructionExpression);

        return children;
    }

    auto Attribute::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Attribute>
    {
        return std::make_shared<const Node::Attribute>(
            m_StructConstructionExpression->CloneInScope(t_scope)
        );
    }

    auto Attribute::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Attribute>>
    {
        ACE_TRY(boundStructConstructionExpression, m_StructConstructionExpression->CreateBound());
        return std::make_shared<const BoundNode::Attribute>(boundStructConstructionExpression);
    }
}
