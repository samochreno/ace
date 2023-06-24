#include "Node/Attribute.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Attribute.hpp"

namespace Ace::Node
{
    Attribute::Attribute(
        const std::shared_ptr<const Node::Expr::StructConstruction>& t_structConstructionExpr
    ) : m_StructConstructionExpr{ t_structConstructionExpr }
    {
    }

    auto Attribute::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_StructConstructionExpr->GetScope();
    }

    auto Attribute::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_StructConstructionExpr);

        return children;
    }

    auto Attribute::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Attribute>
    {
        return std::make_shared<const Node::Attribute>(
            m_StructConstructionExpr->CloneInScope(t_scope)
        );
    }

    auto Attribute::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Attribute>>
    {
        ACE_TRY(boundStructConstructionExpr, m_StructConstructionExpr->CreateBound());
        return std::make_shared<const BoundNode::Attribute>(
            boundStructConstructionExpr
        );
    }
}
