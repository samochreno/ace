#include "Node/Statement/Exit.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Error.hpp"
#include "BoundNode/Statement/Exit.hpp"

namespace Ace::Node::Statement
{
    auto Exit::GetChildren() const -> std::vector<const Node::IBase*> 
    {
        return {};
    }

    auto Exit::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::Exit>
    {
        return std::make_shared<const Node::Statement::Exit>(t_scope);
    }

    auto Exit::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Exit>>
    {
        return std::make_shared<const BoundNode::Statement::Exit>(m_Scope);
    }
}
