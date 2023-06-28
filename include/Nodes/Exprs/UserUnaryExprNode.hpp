#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNode/Expr/UserUnary.hpp"
#include "Token.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class UserUnaryExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<UserUnaryExprNode>,
        public virtual IBindableNode<BoundNode::Expr::UserUnary>
    {
    public:
        UserUnaryExprNode(
            const std::shared_ptr<const IExprNode>& t_expr,
            const TokenKind& t_operator
        );
        virtual ~UserUnaryExprNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const UserUnaryExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::UserUnary>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final;
            
    private:
        std::shared_ptr<const IExprNode> m_Expr{};
        TokenKind m_Operator{};
    };
}
