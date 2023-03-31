#include "Node/Template/Type.hpp"

#include <memory>
#include <vector>
#include <string>

#include "Scope.hpp"
#include "Error.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Template/Type.hpp"

namespace Ace::Node::Template
{
    auto Type::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto Type::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Template::Type>
    {
        return std::make_unique<const Node::Template::Type>(
            m_Parameters,
            m_AST->CloneInScopeType(t_scope)
            );
    }

    auto Type::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Template::Type>(this)
        };
    }

    auto Type::GetImplParameters() const -> const std::vector<std::string>& 
    {
        static std::vector<std::string> implParameters{};
        return implParameters;
    }
}
