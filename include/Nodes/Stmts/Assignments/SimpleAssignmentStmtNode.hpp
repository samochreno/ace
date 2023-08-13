#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/Assignments/SimpleAssignmentStmtBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class SimpleAssignmentStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableInScopeNode<SimpleAssignmentStmtNode>,
        public virtual IBindableNode<SimpleAssignmentStmtBoundNode>
    {
    public:
        SimpleAssignmentStmtNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::shared_ptr<const IExprNode>& lhsExpr,
            const std::shared_ptr<const IExprNode>& rhsExpr
        );
        virtual ~SimpleAssignmentStmtNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const SimpleAssignmentStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Diagnosed<std::shared_ptr<const SimpleAssignmentStmtBoundNode>> final;
        auto CreateBoundStmt() const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>> final;
        
    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const IExprNode> m_LHSExpr{};
        std::shared_ptr<const IExprNode> m_RHSExpr{};
    };
}
