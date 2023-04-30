#pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression/DerefAs.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Error.hpp"

namespace Ace::Node::Expression
{
    class DerefAs :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::DerefAs>,
        public virtual Node::IBindable<BoundNode::Expression::DerefAs>
    {
    public:
        DerefAs(
            const TypeName& t_typeName, 
            const std::shared_ptr<const Node::Expression::IBase>& t_expression
        ) : m_TypeName{ t_typeName },
            m_Expression{ t_expression }
        {
        }
        virtual ~DerefAs() = default;

        auto GetScope() const -> Scope* final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::DerefAs> final;
        auto CloneInScopeExpression(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::DerefAs>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }
        
    private:
        TypeName m_TypeName{};
        std::shared_ptr<const Node::Expression::IBase> m_Expression{};
    };
}
