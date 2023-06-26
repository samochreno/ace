#pragma once

#include <vector>

#include "Symbols/Symbol.hpp"

namespace Ace
{
    class ITypeSymbol;

    class ITemplatableSymbol : public virtual ISymbol
    {
    public:
        virtual ~ITemplatableSymbol() = default;

        virtual auto CollectTemplateArgs()     const -> std::vector<ITypeSymbol*> = 0;
        virtual auto CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*> = 0;

        virtual auto IsTemplatePlaceholder() const -> bool final;
    };
}
