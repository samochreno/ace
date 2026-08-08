#pragma once

#include <memory>

namespace Ace
{
    class ISymbol;
    class Scope;

    auto BindSymbolParents(const std::shared_ptr<Scope>& scope) -> void;
    auto BindSymbolParents(ISymbol* const parent) -> void;
}
