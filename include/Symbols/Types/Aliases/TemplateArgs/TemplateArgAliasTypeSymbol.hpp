#pragma once

#include "Symbols/Types/Aliases/AliasTypeSymbol.hpp"

namespace Ace
{
    class ITemplateArgAliasTypeSymbol : public virtual IAliasTypeSymbol
    {
    public:
        virtual ~ITemplateArgAliasTypeSymbol() = default;

        virtual auto GetIndex() const -> size_t = 0;
    };
}
