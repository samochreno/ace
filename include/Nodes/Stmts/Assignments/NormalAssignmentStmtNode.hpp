#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/Assignments/NormalAssignmentStmtBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"

namespace Ace
{
    class NormalAssignmentStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<NormalAssignmentStmtNode>,
        public virtual IBindableNode<NormalAssignmentStmtBoundNode>
    {
    public:
        NormalAssignmentStmtNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::shared_ptr<const IExprNode>& lhsExpr,
            const std::shared_ptr<const IExprNode>& rhsExpr
        );
        virtual ~NormalAssignmentStmtNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const NormalAssignmentStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> std::shared_ptr<const NormalAssignmentStmtBoundNode> final;
        auto CreateBoundStmt() const -> std::shared_ptr<const IStmtBoundNode> final;
        
    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const IExprNode> m_LHSExpr{};
        std::shared_ptr<const IExprNode> m_RHSExpr{};
    };
}
