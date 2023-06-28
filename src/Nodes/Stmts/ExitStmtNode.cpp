#include "Nodes/Stmts/ExitStmtNode.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/Exit.hpp"

namespace Ace
{
    ExitStmtNode::ExitStmtNode(
        const std::shared_ptr<Scope>& t_scope
    ) : m_Scope{ t_scope }
    {
    }

    auto ExitStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ExitStmtNode::GetChildren() const -> std::vector<const INode*> 
    {
        return {};
    }

    auto ExitStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const ExitStmtNode>
    {
        return std::make_shared<const ExitStmtNode>(t_scope);
    }

    auto ExitStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(t_scope);
    }

    auto ExitStmtNode::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Exit>>
    {
        return std::make_shared<const BoundNode::Stmt::Exit>(m_Scope);
    }

    auto ExitStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return CreateBound();
    }
}
