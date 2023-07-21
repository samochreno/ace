#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/Assignments/CompoundAssignmentStmtBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Op.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class CompoundAssignmentStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<CompoundAssignmentStmtNode>,
        public virtual IBindableNode<CompoundAssignmentStmtBoundNode>
    {
    public:
        CompoundAssignmentStmtNode(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<const IExprNode>& t_lhsExpr,
            const std::shared_ptr<const IExprNode>& t_rhsExpr,
            const Op& t_op
        );
        virtual ~CompoundAssignmentStmtNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const CompoundAssignmentStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const CompoundAssignmentStmtBoundNode>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>> final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const IExprNode> m_LHSExpr{};
        std::shared_ptr<const IExprNode> m_RHSExpr{};
        Op m_Op{};
    };
}
