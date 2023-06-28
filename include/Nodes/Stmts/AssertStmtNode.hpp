#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNode/Stmt/Assert.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class AssertStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<AssertStmtNode>,
        public virtual IBindableNode<BoundNode::Stmt::Assert>
    {
    public:
        AssertStmtNode(
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<const IExprNode>& t_condition
        );
        virtual ~AssertStmtNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const AssertStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Assert>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const IExprNode> m_Condition{};
    };
}
