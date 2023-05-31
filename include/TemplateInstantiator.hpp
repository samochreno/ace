#pragma once

#include <memory>
#include <map>

#include "Error.hpp"

namespace Ace::Node
{
    class IBase;
}

namespace Ace::Symbol
{
    class IBase;

    namespace Type
    {
        class IBase;
    }

    namespace Template
    {
        class IBase;
    }
}

namespace Ace
{
    class TemplateInstantiator
    {
    public:
        TemplateInstantiator();
        ~TemplateInstantiator();

        auto SetSymbols(
            const std::vector<Symbol::Template::IBase*>& t_symbols
        ) -> void { m_Symbols = t_symbols; }

        auto InstantiatePlaceholderSymbols() -> Expected<void>;

        auto InstantiateSymbols(
            Symbol::Template::IBase* const t_templateSymbol,
            const std::vector<Symbol::Type::IBase*>& t_implArguments,
            const std::vector<Symbol::Type::IBase*>& t_arguments
        ) -> Expected<Symbol::IBase*>;
        auto InstantiateSemanticsForSymbols() -> void;

    private:
        std::vector<Symbol::Template::IBase*> m_Symbols;
        std::map<Symbol::Template::IBase*, std::vector<std::shared_ptr<const Node::IBase>>> m_SymbolOnlyInstantiatedASTsMap;
    };
}
