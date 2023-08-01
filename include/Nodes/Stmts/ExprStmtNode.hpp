#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/ExprStmtBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"

namespace Ace
{
    class ExprStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<ExprStmtNode>,
        public virtual IBindableNode<ExprStmtBoundNode>
    {
    public:
        ExprStmtNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprNode>& expr
        );
        virtual ~ExprStmtNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const ExprStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> std::shared_ptr<const ExprStmtBoundNode> final;
        auto CreateBoundStmt() const -> std::shared_ptr<const IStmtBoundNode> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprNode> m_Expr{};
    };
}
