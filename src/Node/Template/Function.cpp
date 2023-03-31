#include "Node/Template/Function.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Error.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Template/Function.hpp"

namespace Ace::Node::Template
{
    auto Function::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto Function::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Template::Function>
    {
        return std::make_unique<const Node::Template::Function>(
            m_ImplParameters,
            m_Parameters,
            m_AST->CloneInScope(t_scope)
            );
    }

    auto Function::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Template::Function>(this)
        };
    }
}
