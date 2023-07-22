#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/BoxExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class BoxExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<BoxExprNode>,
        public virtual IBindableNode<BoxExprBoundNode>
    {
    public:
        BoxExprNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprNode>& expr
        );
        virtual ~BoxExprNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const BoxExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoxExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;
    
    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprNode> m_Expr{};
    };
}
