#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/Assignments/NormalAssignmentStmtBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class AssignmentStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<AssignmentStmtNode>,
        public virtual IBindableNode<AssignmentStmtBoundNode>
    {
    public:
        AssignmentStmtNode(
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<const IExprNode>& t_lhsExpr,
            const std::shared_ptr<const IExprNode>& t_rhsExpr
        );
        virtual ~AssignmentStmtNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const AssignmentStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const AssignmentStmtBoundNode>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>> final;
        
    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const IExprNode> m_LHSExpr{};
        std::shared_ptr<const IExprNode> m_RHSExpr{};
    };
}
