#pragma once

#include <memory>
#include <vector>
#include <string>

#include "SymbolKind.hpp"
#include "SymbolCategory.hpp"
#include "AccessModifier.hpp"
#include "Name.hpp"

namespace Ace
{
    class Scope;
}

namespace Ace::Symbol
{
    class IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetScope() const -> Scope* = 0;
        virtual auto GetName() const -> const std::string& = 0;
        virtual auto GetSymbolKind() const -> SymbolKind = 0;
        virtual auto GetSymbolCategory() const -> SymbolCategory = 0;
        virtual auto GetAccessModifier() const -> AccessModifier = 0;

        virtual auto CreatePartialSignature() const -> std::string final;
        virtual auto CreateSignature() const -> std::string final;
        virtual auto GetFullyQualifiedName() const -> Name::Symbol::Full final;
    };
}
