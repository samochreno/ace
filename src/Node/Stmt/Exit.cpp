#include "Node/Stmt/Exit.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/Exit.hpp"

namespace Ace::Node::Stmt
{
    Exit::Exit(
        const std::shared_ptr<Scope>& t_scope
    ) : m_Scope{ t_scope }
    {
    }

    auto Exit::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Exit::GetChildren() const -> std::vector<const Node::IBase*> 
    {
        return {};
    }

    auto Exit::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::Exit>
    {
        return std::make_shared<const Node::Stmt::Exit>(t_scope);
    }

    auto Exit::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto Exit::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Exit>>
    {
        return std::make_shared<const BoundNode::Stmt::Exit>(m_Scope);
    }

    auto Exit::CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return CreateBound();
    }
}
