#pragma once

#include <memory>
#include <map>

#include "Diagnostic.hpp"

namespace Ace
{
    class ITemplatableNode;
    class ISymbol;
    class ITypeSymbol;
    class ITemplateSymbol;

    class TemplateInstantiator
    {
    public:
        TemplateInstantiator();
        ~TemplateInstantiator();

        auto SetSymbols(
            const std::vector<ITemplateSymbol*>& symbols
        ) -> void;

        auto InstantiatePlaceholderSymbols() -> Diagnosed<void>;

        auto InstantiateSymbols(
            ITemplateSymbol* const templateSymbol,
            const std::vector<ITypeSymbol*>& implArgs,
            const std::vector<ITypeSymbol*>& args
        ) -> Diagnosed<ISymbol*>;
        auto InstantiateSemanticsForSymbols() -> void;

    private:
        std::vector<ITemplateSymbol*> m_Symbols;
        std::map<ITemplateSymbol*, std::vector<std::shared_ptr<const ITemplatableNode>>> m_SymbolOnlyInstantiatedASTsMap;
    };
}
