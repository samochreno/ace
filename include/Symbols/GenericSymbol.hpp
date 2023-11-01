#pragma once

#include <memory>
#include <vector>

#include "Symbols/Symbol.hpp"
#include "Symbols/BodyScopedSymbol.hpp"
#include "Scope.hpp"

namespace Ace
{
    class ITypeSymbol;
    class TypeParamTypeSymbol;

    class IGenericSymbol :
        public virtual ISymbol,
        public virtual IBodyScopedSymbol
    {
    public:
        virtual ~IGenericSymbol() = default;

        virtual auto SetBodyScope(
            const std::shared_ptr<Scope>& scope
        ) -> void = 0;

        virtual auto GetTypeArgs() const -> const std::vector<ITypeSymbol*>& = 0;
        virtual auto CollectTypeParams() const -> std::vector<TypeParamTypeSymbol*> final;

        virtual auto IsPlaceholder() const -> bool final;

        virtual auto GetGenericRoot() const -> IGenericSymbol* final;

        virtual auto IsInstance() const -> bool final;
    };
}
