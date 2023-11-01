#pragma once

#include <memory>
#include <vector>

#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"

namespace Ace
{
    class IAliasTypeSymbol : public virtual ITypeSymbol
    {
    public:
        virtual ~IAliasTypeSymbol() = default;

        virtual auto GetAliasedType() const -> ITypeSymbol* = 0;

        auto SetBodyScope(const std::shared_ptr<Scope>& scope) -> void final;
        auto GetTypeArgs() const -> const std::vector<ITypeSymbol*>& final;
    };
}
