#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/UnboxExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class UnboxExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<UnboxExprNode>,
        public virtual IBindableNode<UnboxExprBoundNode>
    {
    public:
        UnboxExprNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprNode>& expr
        );
        virtual ~UnboxExprNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const UnboxExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const UnboxExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprNode> m_Expr{};
    };
}
