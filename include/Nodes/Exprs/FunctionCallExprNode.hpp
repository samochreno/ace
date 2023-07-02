#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class FunctionCallExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<FunctionCallExprNode>,
        public virtual IBindableNode<IExprBoundNode>
    {
    public:
        FunctionCallExprNode(
            const std::shared_ptr<const IExprNode>& t_expr,
            const std::vector<std::shared_ptr<const IExprNode>>& t_args
        );
        virtual ~FunctionCallExprNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const FunctionCallExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;

    private:
        std::shared_ptr<const IExprNode> m_Expr{};
        std::vector<std::shared_ptr<const IExprNode>> m_Args{};
    };
}
