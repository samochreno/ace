#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Identifier.hpp"

namespace Ace
{
    class Scope;

    class ISymbolCreatable
    {
    public:
        virtual ~ISymbolCreatable() = default;

        virtual auto GetSymbolScope() const -> std::shared_ptr<Scope> = 0;
        virtual auto GetSymbolKind() const -> SymbolKind = 0;
        virtual auto GetSymbolCreationSuborder() const -> size_t = 0;
        virtual auto CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>> = 0;
    };

    class IPartiallySymbolCreatable : public virtual ISymbolCreatable
    {
    public:
        virtual ~IPartiallySymbolCreatable() = default;

        virtual auto GetName() const -> const Identifier& = 0;
        virtual auto ContinueCreatingSymbol(
            ISymbol* const t_symbol
        ) const -> Expected<void> = 0;
    };
}
