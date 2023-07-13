#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class LogicalNegationExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<LogicalNegationExprNode>, 
        public virtual IBindableNode<LogicalNegationExprBoundNode>
    {
    public:
        LogicalNegationExprNode(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<const IExprNode>& t_expr
        );
        virtual ~LogicalNegationExprNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope>;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const LogicalNegationExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const LogicalNegationExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<const IExprNode> m_Expr{};
    };
}
