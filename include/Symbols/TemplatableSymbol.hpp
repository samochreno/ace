#pragma once

#include <vector>

#include "Symbols/Symbol.hpp"
#include "Symbols/SelfScopedSymbol.hpp"

namespace Ace
{
    class ITypeSymbol;

    class ITemplatableSymbol :
        public virtual ISymbol,
        public virtual ISelfScopedSymbol
    {
    public:
        virtual ~ITemplatableSymbol() = default;

        virtual auto CollectTemplateArgs()     const -> std::vector<ITypeSymbol*> final;
        virtual auto CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*> final;

        virtual auto IsTemplatePlaceholder() const -> bool final;
    };
}
