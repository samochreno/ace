#pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression//Or.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Expression
{
    class Or :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::Or>,
        public virtual Node::IBindable<BoundNode::Expression::Or>
    {
    public:
        Or(
            const std::shared_ptr<const Node::Expression::IBase>& t_lhsExpression,
            const std::shared_ptr<const Node::Expression::IBase>& t_rhsExpression
        ) : m_LHSExpression{ t_lhsExpression },
            m_RHSExpression{ t_rhsExpression }
        {
        }
        virtual ~Or() = default;

        auto GetScope() const -> Scope* final { return m_LHSExpression->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::Or> final;
        auto CloneInScopeExpression(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::Or>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<const Node::Expression::IBase> m_LHSExpression{};
        std::shared_ptr<const Node::Expression::IBase> m_RHSExpression{};
    };
}
