#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Symbols/Symbol.hpp"
#include "Symbols/SelfScopedSymbol.hpp"
#include "Symbols/TemplatableSymbol.hpp"
#include "Diagnostic.hpp"
#include "Emittable.hpp"

namespace Ace
{
    class FunctionSymbol;
    class TypeTemplateSymbol;

    class ITypeSymbol :
        public virtual ISymbol,
        public virtual ISelfScopedSymbol,
        public virtual ITemplatableSymbol
    {
    public:
        virtual ~ITypeSymbol() = default;

        virtual auto DiagnoseCycle() const -> Diagnosed<void>;

        virtual auto IsRef() const -> bool final;
        virtual auto GetWithoutRef() -> ITypeSymbol* final;
        virtual auto GetWithoutRef() const -> const ITypeSymbol* final;
        virtual auto GetWithRef() -> ITypeSymbol* final;
        virtual auto IsStrongPtr() const -> bool final;
        virtual auto GetWithoutStrongPtr() -> ITypeSymbol* final;
        virtual auto GetWithStrongPtr() -> ITypeSymbol* final;
        virtual auto IsWeakPtr() const -> bool final;
        virtual auto GetWithoutWeakPtr() -> ITypeSymbol* final;
        virtual auto GetWithWeakPtr() -> ITypeSymbol* final;
        virtual auto GetUnaliased() -> ITypeSymbol* final;
        virtual auto GetUnaliased() const -> const ITypeSymbol* final;
        virtual auto GetTemplate() const -> std::optional<TypeTemplateSymbol*> final;
    };
}
