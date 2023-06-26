#pragma once

#include <memory>
#include <map>

#include "Diagnostics.hpp"

namespace Ace::Node
{
    class IBase;
}

namespace Ace
{
    class ISymbol;
    class ITypeSymbol;
    class ITemplateSymbol;

    class TemplateInstantiator
    {
    public:
        TemplateInstantiator();
        ~TemplateInstantiator();

        auto SetSymbols(
            const std::vector<ITemplateSymbol*>& t_symbols
        ) -> void;

        auto InstantiatePlaceholderSymbols() -> Expected<void>;

        auto InstantiateSymbols(
            ITemplateSymbol* const t_templateSymbol,
            const std::vector<ITypeSymbol*>& t_implArgs,
            const std::vector<ITypeSymbol*>& t_args
        ) -> Expected<ISymbol*>;
        auto InstantiateSemanticsForSymbols() -> void;

    private:
        std::vector<ITemplateSymbol*> m_Symbols;
        std::map<ITemplateSymbol*, std::vector<std::shared_ptr<const Node::IBase>>> m_SymbolOnlyInstantiatedASTsMap;
    };
}
