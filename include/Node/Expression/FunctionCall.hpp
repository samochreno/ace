#pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Expression
{
    class FunctionCall :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::FunctionCall>,
        public virtual Node::IBindable<BoundNode::Expression::IBase>
    {
    public:
        FunctionCall(
            const std::shared_ptr<const Node::Expression::IBase>& t_expression,
            const std::vector<std::shared_ptr<const Node::Expression::IBase>>& t_arguments
        ) : m_Expression{ t_expression },
            m_Arguments{ t_arguments }
        {
        }
        virtual ~FunctionCall() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::FunctionCall> final;
        auto CloneInScopeExpression(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<const Node::Expression::IBase> m_Expression{};
        std::vector<std::shared_ptr<const Node::Expression::IBase>> m_Arguments{};
    };
}
