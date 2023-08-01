#include "Nodes/Stmts/ExitStmtNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/ExitStmtBoundNode.hpp"

namespace Ace
{
    ExitStmtNode::ExitStmtNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope }
    {
    }

    auto ExitStmtNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ExitStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ExitStmtNode::CollectChildren() const -> std::vector<const INode*> 
    {
        return {};
    }

    auto ExitStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const ExitStmtNode>
    {
        return std::make_shared<const ExitStmtNode>(
            m_SrcLocation,
            scope
        );
    }

    auto ExitStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto ExitStmtNode::CreateBound() const -> std::shared_ptr<const ExitStmtBoundNode>
    {
        return std::make_shared<const ExitStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope()
        );
    }

    auto ExitStmtNode::CreateBoundStmt() const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateBound();
    }
}
