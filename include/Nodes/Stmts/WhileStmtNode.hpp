#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "BoundNodes/Stmts/WhileStmtBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class WhileStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<WhileStmtNode>,
        public virtual IBindableNode<WhileStmtBoundNode>
    {
    public:
        WhileStmtNode(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<const IExprNode>& t_condition,
            const std::shared_ptr<const BlockStmtNode>& t_body
        );
        virtual ~WhileStmtNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const WhileStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const WhileStmtBoundNode>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>> final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const IExprNode> m_Condition{};
        std::shared_ptr<const BlockStmtNode> m_Body{};
    };
}
