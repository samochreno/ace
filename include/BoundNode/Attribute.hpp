#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Base.hpp"
#include "BoundNode/Expression/StructConstruction.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode
{
    class Attribute : 
        public std::enable_shared_from_this<BoundNode::Attribute>,
        public virtual BoundNode::IBase, 
        public virtual BoundNode::ITypeCheckable<BoundNode::Attribute>, 
        public virtual BoundNode::ILowerable<BoundNode::Attribute>
    {
    public:
        Attribute(const std::shared_ptr<const BoundNode::Expression::StructConstruction>& t_structConstructionExpression) 
            : m_StructConstructionExpression{ t_structConstructionExpression }
        {
        }
        virtual ~Attribute() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_StructConstructionExpression->GetScope(); }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Attribute>>> final;
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Attribute>> final;

    private:
        std::shared_ptr<const BoundNode::Expression::StructConstruction> m_StructConstructionExpression{};
    };
}
