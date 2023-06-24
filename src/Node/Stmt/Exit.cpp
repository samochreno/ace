#include "Node/Stmt/Exit.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/Exit.hpp"

namespace Ace::Node::Stmt
{
    auto Exit::GetChildren() const -> std::vector<const Node::IBase*> 
    {
        return {};
    }

    auto Exit::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::Exit>
    {
        return std::make_shared<const Node::Stmt::Exit>(t_scope);
    }

    auto Exit::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Exit>>
    {
        return std::make_shared<const BoundNode::Stmt::Exit>(m_Scope);
    }
}
