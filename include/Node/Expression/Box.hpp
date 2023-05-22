#pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression/Box.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Expression
{
    class Box :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::Box>,
        public virtual Node::IBindable<BoundNode::Expression::Box>
    {
    public:
        Box(const std::shared_ptr<const Node::Expression::IBase>& t_expression) 
            : m_Expression{ t_expression }
        {
        }
        virtual ~Box() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::Box> final;
        auto CloneInScopeExpression(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::Box>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }
    
    private:
        std::shared_ptr<const Node::Expression::IBase> m_Expression{};
    };
}
