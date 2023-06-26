#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Compilation.hpp"
#include "Scope.hpp"
#include "SymbolKind.hpp"
#include "SymbolCategory.hpp"
#include "AccessModifier.hpp"
#include "Name.hpp"

namespace Ace
{
    class ISymbol
    {
    public:
        virtual ~ISymbol() = default;

        virtual auto GetCompilation() const -> const Compilation* final;
        virtual auto GetScope() const -> std::shared_ptr<Scope> = 0;
        virtual auto GetName() const -> const std::string& = 0;
        virtual auto GetSymbolKind() const -> SymbolKind = 0;
        virtual auto GetSymbolCategory() const -> SymbolCategory = 0;
        virtual auto GetAccessModifier() const -> AccessModifier = 0;

        virtual auto CreatePartialSignature() const -> std::string final;
        virtual auto CreateSignature() const -> std::string final;
        virtual auto CreateFullyQualifiedName() const -> SymbolName final;
    };
}
