#pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression/UserBinary.hpp"
#include "Scope.hpp"
#include "Token.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expression
{
    class UserBinary :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::UserBinary>,
        public virtual Node::IBindable<BoundNode::Expression::UserBinary>
    {
    public:
        UserBinary(
            const std::shared_ptr<const Node::Expression::IBase>& t_lhsExpression,
            const std::shared_ptr<const Node::Expression::IBase>& t_rhsExpression,
            const TokenKind& t_operator
        ) : m_LHSExpression{ t_lhsExpression },
            m_RHSExpression{ t_rhsExpression },
            m_Operator{ t_operator }
        {
        }
        virtual ~UserBinary() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_LHSExpression->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::UserBinary> final;
        auto CloneInScopeExpression(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::UserBinary>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<const Node::Expression::IBase> m_LHSExpression{};
        std::shared_ptr<const Node::Expression::IBase> m_RHSExpression{};
        TokenKind m_Operator{};
    };
}
