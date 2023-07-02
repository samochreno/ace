#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "BoundNodes/Stmts/ExitStmtBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class ExitStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<ExitStmtNode>,
        public virtual IBindableNode<ExitStmtBoundNode>
    {
    public:
        ExitStmtNode(const std::shared_ptr<Scope>& t_scope);
        virtual ~ExitStmtNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const ExitStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const ExitStmtBoundNode>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
    };
}
