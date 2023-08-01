#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/UserUnaryExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"

namespace Ace
{
    class UserUnaryExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<UserUnaryExprNode>,
        public virtual IBindableNode<UserUnaryExprBoundNode>
    {
    public:
        UserUnaryExprNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprNode>& expr,
            const Op op
        );
        virtual ~UserUnaryExprNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const UserUnaryExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> std::shared_ptr<const UserUnaryExprBoundNode> final;
        auto CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode> final;
            
    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprNode> m_Expr{};
        Op m_Op{};
    };
}
