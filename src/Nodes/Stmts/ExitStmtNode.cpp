#include "Nodes/Stmts/ExitStmtNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/ExitStmtBoundNode.hpp"

namespace Ace
{
    ExitStmtNode::ExitStmtNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope }
    {
    }

    auto ExitStmtNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const ExitStmtNode>
    {
        return std::make_shared<const ExitStmtNode>(
            m_SourceLocation,
            scope
        );
    }

    auto ExitStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto ExitStmtNode::CreateBound() const -> Expected<std::shared_ptr<const ExitStmtBoundNode>>
    {
        return std::make_shared<const ExitStmtBoundNode>(
            GetSourceLocation(),
            GetScope()
        );
    }

    auto ExitStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
