#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "BoundNodes/Stmts/WhileStmtBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"

namespace Ace
{
    class WhileStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableInScopeNode<WhileStmtNode>,
        public virtual IBindableNode<WhileStmtBoundNode>
    {
    public:
        WhileStmtNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::shared_ptr<const IExprNode>& condition,
            const std::shared_ptr<const BlockStmtNode>& body
        );
        virtual ~WhileStmtNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const WhileStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> std::shared_ptr<const WhileStmtBoundNode> final;
        auto CreateBoundStmt() const -> std::shared_ptr<const IStmtBoundNode> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const IExprNode> m_Condition{};
        std::shared_ptr<const BlockStmtNode> m_Body{};
    };
}
