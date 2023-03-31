#pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression//LogicalNegation.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Expression
{
    class LogicalNegation :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::LogicalNegation>, 
        public virtual Node::IBindable<BoundNode::Expression::LogicalNegation>
    {
    public:
        LogicalNegation(const std::shared_ptr<const Node::Expression::IBase>& t_expression)
            : m_Expression{ t_expression }
        {
        }
        virtual ~LogicalNegation() = default;

        auto GetScope() const -> Scope* final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::LogicalNegation> final;
        auto CloneInScopeExpression(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::LogicalNegation>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<const Node::Expression::IBase> m_Expression{};
    };
}
