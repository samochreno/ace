#include "Node/Stmt/Label.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/Label.hpp"
#include "Symbol/Label.hpp"
#include "Symbol/Base.hpp"

namespace Ace::Node::Stmt
{
    auto Label::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto Label::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::Label>
    {
        return std::make_shared<const Node::Stmt::Label>(
            t_scope,
            m_Name
        );
    }

    auto Label::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Label>>
    {
        auto* const selfSymbol = m_Scope->ExclusiveResolveSymbol<Symbol::Label>(m_Name).Unwrap();
        return std::make_shared<const BoundNode::Stmt::Label>(selfSymbol);
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
