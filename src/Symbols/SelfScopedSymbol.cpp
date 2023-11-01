#include "Symbols/BodyScopedSymbol.hpp"

#include <memory>

#include "Scope.hpp"

namespace Ace
{
    auto IBodyScopedSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return GetBodyScope()->GetParent().value();
    }
}
