#pragma once

#include <memory>

#include "Symbols/Symbol.hpp"

namespace Ace
{
    class IBodyScopedSymbol : public virtual ISymbol
    {
    public:
        virtual ~IBodyScopedSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> override;
        virtual auto GetBodyScope() const -> std::shared_ptr<Scope> = 0;
    };
}
