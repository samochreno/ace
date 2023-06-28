#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNode/Expr//AddressOf.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class AddressOfExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<AddressOfExprNode>,
        public virtual IBindableNode<BoundNode::Expr::AddressOf>
    {
    public:
        AddressOfExprNode(
            const std::shared_ptr<const IExprNode>& t_expr
        );
        virtual ~AddressOfExprNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const AddressOfExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::AddressOf>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final;

    private:
        std::shared_ptr<const IExprNode> m_Expr{};
    };
}
