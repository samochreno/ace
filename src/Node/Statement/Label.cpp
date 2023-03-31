#include "Node/Statement/Label.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Error.hpp"
#include "BoundNode/Statement/Label.hpp"
#include "Symbol/Label.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Statement
{
    auto Label::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto Label::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::Label>
    {
        return std::make_unique<const Node::Statement::Label>(
            t_scope,
            m_Name
            );
    }

    auto Label::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Label>>
    {
        auto* const selfSymbol = m_Scope->ExclusiveResolveSymbol<Symbol::Label>(m_Name).Unwrap();
        return std::make_shared<const BoundNode::Statement::Label>(selfSymbol);
    }

    auto Label::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Label>(
                m_Scope, 
                m_Name
            )
        };
    }
}
