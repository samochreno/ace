#include "Symbols/Types/Aliases/AliasTypeSymbol.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"

namespace Ace
{
    auto IAliasTypeSymbol::SetBodyScope(
        const std::shared_ptr<Scope>& scope
    ) -> void
    {
        GetAliasedType()->SetBodyScope(scope);
    }

    auto IAliasTypeSymbol::GetTypeArgs() const -> const std::vector<ITypeSymbol*>&
    {
        return GetUnaliasedType()->GetTypeArgs();
    }
}
