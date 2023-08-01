#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"

namespace Ace
{
    class FunctionCallExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<FunctionCallExprNode>,
        public virtual IBindableNode<IExprBoundNode>
    {
    public:
        FunctionCallExprNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprNode>& expr,
            const std::vector<std::shared_ptr<const IExprNode>>& args
        );
        virtual ~FunctionCallExprNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const FunctionCallExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> std::shared_ptr<const IExprBoundNode> final;
        auto CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprNode> m_Expr{};
        std::vector<std::shared_ptr<const IExprNode>> m_Args{};
    };
}
