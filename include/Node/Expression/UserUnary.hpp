#pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression/UserUnary.hpp"
#include "Token.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expression
{
    class UserUnary :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::UserUnary>,
        public virtual Node::IBindable<BoundNode::Expression::UserUnary>
    {
    public:
        UserUnary(
            const std::shared_ptr<const Node::Expression::IBase>& t_expression,
            const TokenKind& t_operator
        ) : m_Expression{ t_expression },
            m_Operator{ t_operator }
        {
        }
        virtual ~UserUnary() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::UserUnary> final;
        auto CloneInScopeExpression(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::UserUnary>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }
            
    private:
        std::shared_ptr<const Node::Expression::IBase> m_Expression{};
        TokenKind m_Operator{};
    };
}
