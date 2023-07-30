#pragma once

#include <memory>

#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Ident.hpp"

namespace Ace
{
    class ISymbolCreatable
    {
    public:
        virtual ~ISymbolCreatable() = default;

        virtual auto GetSymbolScope() const -> std::shared_ptr<Scope> = 0;
        virtual auto GetSymbolKind() const -> SymbolKind = 0;
        virtual auto GetSymbolCreationSuborder() const -> size_t = 0;
        virtual auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> = 0;
    };

    class IPartiallySymbolCreatable : public virtual ISymbolCreatable
    {
    public:
        virtual ~IPartiallySymbolCreatable() = default;

        virtual auto GetName() const -> const Ident& = 0;
        virtual auto ContinueCreatingSymbol(
            ISymbol* const symbol
        ) const -> Diagnosed<void> = 0;
    };
}
