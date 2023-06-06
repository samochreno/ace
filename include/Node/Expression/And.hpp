 #pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression//And.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expression
{
    class And :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::And>,
        public virtual Node::IBindable<BoundNode::Expression::And>
    {
    public:
        And(
            const std::shared_ptr<const Node::Expression::IBase>& t_lhsExpression,
            const std::shared_ptr<const Node::Expression::IBase>& t_rhsExpression
        ) : m_LHSExpression{ t_lhsExpression },
            m_RHSExpression{ t_rhsExpression }
        {
        }
        virtual ~And() = default;
    
        auto GetScope() const -> std::shared_ptr<Scope> final { return m_LHSExpression->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::And> final;
        auto CloneInScopeExpression(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::And>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<const Node::Expression::IBase> m_LHSExpression{};
        std::shared_ptr<const Node::Expression::IBase> m_RHSExpression{};
    };
}
