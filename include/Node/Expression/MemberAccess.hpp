#pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression/VariableReference/Instance.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Error.hpp"

namespace Ace::Node::Expression
{
    class MemberAccess :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::MemberAccess>,
        public virtual Node::IBindable<BoundNode::Expression::VariableReference::Instance>
    {
    public:
        MemberAccess(
            const std::shared_ptr<const Node::Expression::IBase>& t_expression,
            const SymbolNameSection& t_name
        ) : m_Expression{ t_expression },
            m_Name{ t_name }
        {
        }
        virtual ~MemberAccess() = default;

        auto GetScope() const -> Scope* final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::MemberAccess> final;
        auto CloneInScopeExpression(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::VariableReference::Instance>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }

        auto GetExpression() const -> const Node::Expression::IBase* { return m_Expression.get(); }
        auto GetName() const -> const SymbolNameSection& { return m_Name; }

    private:
        std::shared_ptr<const Node::Expression::IBase> m_Expression{};
        SymbolNameSection m_Name{};
    };
}
