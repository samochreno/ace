#pragma once

#include <memory>
#include <vector>

#include "Node/Expr/Base.hpp"
#include "BoundNode/Expr/UserUnary.hpp"
#include "Token.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expr
{
    class UserUnary :
        public virtual Node::Expr::IBase,
        public virtual Node::ICloneable<Node::Expr::UserUnary>,
        public virtual Node::IBindable<BoundNode::Expr::UserUnary>
    {
    public:
        UserUnary(
            const std::shared_ptr<const Node::Expr::IBase>& t_expr,
            const TokenKind& t_operator
        ) : m_Expr{ t_expr },
            m_Operator{ t_operator }
        {
        }
        virtual ~UserUnary() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Expr->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::UserUnary> final;
        auto CloneInScopeExpr(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::UserUnary>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final { return CreateBound(); }
            
    private:
        std::shared_ptr<const Node::Expr::IBase> m_Expr{};
        TokenKind m_Operator{};
    };
}
