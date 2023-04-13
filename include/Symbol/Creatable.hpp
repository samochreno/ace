#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include "Symbol/Base.hpp"
#include "Error.hpp"

namespace Ace::Symbol
{
    class ICreatable
    {
    public:
        virtual ~ICreatable() = default;

        virtual auto GetSymbolScope() const -> Scope* = 0;
        virtual auto GetSymbolKind() const -> SymbolKind = 0;
        virtual auto GetSymbolCreationSuborder() const -> size_t = 0;
        virtual auto CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>> = 0;
    };

    class IPartiallyCreatable : public virtual ICreatable
    {
    public:
        virtual ~IPartiallyCreatable() = default;

        virtual auto GetName() const -> const std::string& = 0;
        virtual auto ContinueCreatingSymbol(Symbol::IBase* const t_symbol) const -> Expected<void> = 0;
    };
}
