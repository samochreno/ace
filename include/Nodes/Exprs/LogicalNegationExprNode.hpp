#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNode/Expr//LogicalNegation.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class LogicalNegationExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<LogicalNegationExprNode>, 
        public virtual IBindableNode<BoundNode::Expr::LogicalNegation>
    {
    public:
        LogicalNegationExprNode(
            const std::shared_ptr<const IExprNode>& t_expr
        );
        virtual ~LogicalNegationExprNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope>;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const LogicalNegationExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::LogicalNegation>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final;

    private:
        std::shared_ptr<const IExprNode> m_Expr{};
    };
}
