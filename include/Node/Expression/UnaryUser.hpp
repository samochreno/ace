#pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression/UnaryUser.hpp"
#include "Token.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Expression
{
    class UnaryUser :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::UnaryUser>,
        public virtual Node::IBindable<BoundNode::Expression::UnaryUser>
    {
    public:
        UnaryUser(
            const std::shared_ptr<const Node::Expression::IBase>& t_expression,
            const Token::Kind::Set& t_operator
        ) : m_Expression{ t_expression },
            m_Operator{ t_operator }
        {
        }
        virtual ~UnaryUser() = default;

        auto GetScope() const -> Scope* final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::UnaryUser> final;
        auto CloneInScopeExpression(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::UnaryUser>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }
            
    private:
        std::shared_ptr<const Node::Expression::IBase> m_Expression{};
        Token::Kind::Set m_Operator{};
    };
}
