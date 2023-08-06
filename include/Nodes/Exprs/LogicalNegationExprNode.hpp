#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class LogicalNegationExprNode :
        public virtual IExprNode,
        public virtual ICloneableInScopeNode<LogicalNegationExprNode>, 
        public virtual IBindableNode<LogicalNegationExprBoundNode>
    {
    public:
        LogicalNegationExprNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprNode>& expr
        );
        virtual ~LogicalNegationExprNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope>;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const LogicalNegationExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Diagnosed<std::shared_ptr<const LogicalNegationExprBoundNode>> final;
        auto CreateBoundExpr() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprNode> m_Expr{};
    };
}
