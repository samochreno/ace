#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/UserBinaryExprBoundNode.hpp"
#include "Scope.hpp"
#include "Token.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class UserBinaryExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<UserBinaryExprNode>,
        public virtual IBindableNode<UserBinaryExprBoundNode>
    {
    public:
        UserBinaryExprNode(
            const std::shared_ptr<const IExprNode>& t_lhsExpr,
            const std::shared_ptr<const IExprNode>& t_rhsExpr,
            const TokenKind& t_operator
        );
        virtual ~UserBinaryExprNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const UserBinaryExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const UserBinaryExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;

    private:
        std::shared_ptr<const IExprNode> m_LHSExpr{};
        std::shared_ptr<const IExprNode> m_RHSExpr{};
        TokenKind m_Operator{};
    };
}
