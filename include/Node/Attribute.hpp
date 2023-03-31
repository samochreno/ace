#pragma once

#include <memory>
#include <vector>

#include "Node/Base.hpp"
#include "Node/Expression/StructConstruction.hpp"
#include "BoundNode/Attribute.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node
{
    class Attribute :
        public virtual Node::IBase,
        public virtual Node::ICloneable<Node::Attribute>,
        public virtual Node::IBindable<BoundNode::Attribute>
    {
    public:
        Attribute(const std::shared_ptr<const Node::Expression::StructConstruction>& t_structConstructionExpression) 
            : m_StructConstructionExpression{ t_structConstructionExpression }
        {
        }
        virtual ~Attribute() = default;

        auto GetScope() const -> Scope* final { return m_StructConstructionExpression->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Attribute> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Attribute>> final;

    private:
        std::shared_ptr<const Node::Expression::StructConstruction> m_StructConstructionExpression{};
    };
}
