#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNode/Expr/Unbox.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class UnboxExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<UnboxExprNode>,
        public virtual IBindableNode<BoundNode::Expr::Unbox>
    {
    public:
        UnboxExprNode(const std::shared_ptr<const IExprNode>& t_expr);
        virtual ~UnboxExprNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const UnboxExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::Unbox>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final;

    private:
        std::shared_ptr<const IExprNode> m_Expr{};
    };
}
