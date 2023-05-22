#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include "Error.hpp"
#include "Symbol/Base.hpp"

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
        virtual auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> = 0;
    };

    class IPartiallySymbolCreatable : public virtual ISymbolCreatable
    {
    public:
        virtual ~IPartiallySymbolCreatable() = default;

        virtual auto GetName() const -> const std::string& = 0;
        virtual auto ContinueCreatingSymbol(Symbol::IBase* const t_symbol) const -> Expected<void> = 0;
    };
}
