#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/UserUnaryExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"
#include "TokenKind.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class UserUnaryExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<UserUnaryExprNode>,
        public virtual IBindableNode<UserUnaryExprBoundNode>
    {
    public:
        UserUnaryExprNode(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<const IExprNode>& t_expr,
            const Op t_op
        );
        virtual ~UserUnaryExprNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const UserUnaryExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const UserUnaryExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;
            
    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<const IExprNode> m_Expr{};
        Op m_Op{};
    };
}
