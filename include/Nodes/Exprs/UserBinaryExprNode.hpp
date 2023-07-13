#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/UserBinaryExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "TokenKind.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class UserBinaryExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<UserBinaryExprNode>,
        public virtual IBindableNode<UserBinaryExprBoundNode>
    {
    public:
        UserBinaryExprNode(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<const IExprNode>& t_lhsExpr,
            const std::shared_ptr<const IExprNode>& t_rhsExpr,
            const TokenKind t_operator
        );
        virtual ~UserBinaryExprNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
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
        SourceLocation m_SourceLocation{};
        std::shared_ptr<const IExprNode> m_LHSExpr{};
        std::shared_ptr<const IExprNode> m_RHSExpr{};
        TokenKind m_Operator{};
    };
}
