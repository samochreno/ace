#pragma once

#include <memory>
#include <map>

#include "Diagnostics.hpp"

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
        ) -> void;

        auto InstantiatePlaceholderSymbols() -> Expected<void>;

        auto InstantiateSymbols(
            Symbol::Template::IBase* const t_templateSymbol,
            const std::vector<Symbol::Type::IBase*>& t_implArgs,
            const std::vector<Symbol::Type::IBase*>& t_args
        ) -> Expected<Symbol::IBase*>;
        auto InstantiateSemanticsForSymbols() -> void;

    private:
        std::vector<Symbol::Template::IBase*> m_Symbols;
        std::map<Symbol::Template::IBase*, std::vector<std::shared_ptr<const Node::IBase>>> m_SymbolOnlyInstantiatedASTsMap;
    };
}
