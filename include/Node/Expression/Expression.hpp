#pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression/Expression.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Expression
{
    class Expression :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::Expression>,
        public virtual Node::IBindable<BoundNode::Expression::Expression>
    {
    public:
        Expression(const std::shared_ptr<const Node::Expression::IBase>& t_expression) 
            : m_Expression{ t_expression }
        {
        }
        virtual ~Expression() = default;

        auto GetScope() const -> Scope* final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::Expression> final;
        auto CloneInScopeExpression(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::Expression>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<const Node::Expression::IBase> m_Expression{};
    };
}
