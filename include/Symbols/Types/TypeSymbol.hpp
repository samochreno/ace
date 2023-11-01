#pragma once

#include <vector>

#include "Symbols/Symbol.hpp"
#include "Symbols/AccessibleBodyScopedSymbol.hpp"
#include "Symbols/GenericSymbol.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class TraitTypeSymbol;

    class ITypeSymbol :
        public virtual ISymbol,
        public virtual IAccessibleBodyScopedSymbol,
        public virtual IGenericSymbol
    {
    public:
        virtual ~ITypeSymbol() = default;

        virtual auto GetUnaliasedType() const -> const ITypeSymbol* final;
        virtual auto GetUnaliasedType()       ->       ITypeSymbol* final;

        virtual auto DiagnoseCycle() const -> Diagnosed<void>;

        virtual auto IsRef() const -> bool final;
        virtual auto GetWithoutRef() -> ITypeSymbol* final;
        virtual auto GetWithoutRef() const -> const ITypeSymbol* final;
        virtual auto GetWithRef() -> ITypeSymbol* final;
        virtual auto IsStrongPtr() const -> bool final;
        virtual auto IsDynStrongPtr() const -> bool final;
        virtual auto IsAnyStrongPtr() const -> bool final;
        virtual auto GetWithoutStrongPtr() -> ITypeSymbol* final;
        virtual auto GetWithStrongPtr() -> ITypeSymbol* final;
        virtual auto GetWithDynStrongPtr() -> ITypeSymbol* final;
        virtual auto GetWithAutoStrongPtr() -> ITypeSymbol* final;
        virtual auto IsWeakPtr() const -> bool final;
        virtual auto GetWithoutWeakPtr() -> ITypeSymbol* final;
        virtual auto GetWithWeakPtr() -> ITypeSymbol* final;
        virtual auto GetDerefed() -> ITypeSymbol* final;
    };
}
