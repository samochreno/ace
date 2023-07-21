#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/UserBinaryExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"
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
            const SourceLocation& sourceLocation,
            const std::shared_ptr<const IExprNode>& lhsExpr,
            const std::shared_ptr<const IExprNode>& rhsExpr,
            const Op& op
        );
        virtual ~UserBinaryExprNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const UserBinaryExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const UserBinaryExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<const IExprNode> m_LHSExpr{};
        std::shared_ptr<const IExprNode> m_RHSExpr{};
        Op m_Op{};
    };
}
