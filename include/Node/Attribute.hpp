#pragma once

#include <memory>
#include <vector>

#include "Node/Base.hpp"
#include "Node/Expr/StructConstruction.hpp"
#include "BoundNode/Attribute.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node
{
    class Attribute :
        public virtual Node::IBase,
        public virtual Node::ICloneable<Node::Attribute>,
        public virtual Node::IBindable<BoundNode::Attribute>
    {
    public:
        Attribute(
            const std::shared_ptr<const Node::Expr::StructConstruction>& t_structConstructionExpr
        );
        virtual ~Attribute() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Attribute> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Attribute>> final;

    private:
        std::shared_ptr<const Node::Expr::StructConstruction> m_StructConstructionExpr{};
    };
}
