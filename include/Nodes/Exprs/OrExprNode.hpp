#pragma once

#include <memory>
#include <vector>

#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Exprs/OrExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class OrExprNode :
        public virtual IExprNode,
        public virtual ICloneableNode<OrExprNode>,
        public virtual IBindableNode<OrExprBoundNode>
    {
    public:
        OrExprNode(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<const IExprNode>& t_lhsExpr,
            const std::shared_ptr<const IExprNode>& t_rhsExpr
        );
        virtual ~OrExprNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const OrExprNode> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const OrExprBoundNode>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<const IExprNode> m_LHSExpr{};
        std::shared_ptr<const IExprNode> m_RHSExpr{};
    };
}
