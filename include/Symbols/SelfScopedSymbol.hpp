#pragma once

#include <memory>

#include "Symbols/Symbol.hpp"

namespace Ace
{
    class ISelfScopedSymbol : public virtual ISymbol
    {
    public:
        virtual ~ISelfScopedSymbol() = default;

        virtual auto GetSelfScope() const -> std::shared_ptr<Scope> = 0;
    };
}
