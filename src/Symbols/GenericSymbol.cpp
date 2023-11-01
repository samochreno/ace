#include "Symbols/GenericSymbol.hpp"

#include <vector>

#include "Symbols/Types/TypeParamTypeSymbol.hpp"
#include "Symbols/PrototypeSymbol.hpp"
#include "GenericInstantiator.hpp"

namespace Ace
{
    auto IGenericSymbol::CollectTypeParams() const -> std::vector<TypeParamTypeSymbol*>
    {
        if (GetTypeArgs().empty())
        {
            return {};
        }

        return GetGenericRoot()->GetBodyScope()->CollectTypeParams();
    }

    static auto IsParam(const ISymbol* const symbol) -> bool
    {
        return
            dynamic_cast<const TypeParamTypeSymbol*>(symbol) !=
            nullptr;
    }

    auto IGenericSymbol::IsPlaceholder() const -> bool
    {
        const auto* const self = dynamic_cast<IGenericSymbol*>(GetUnaliased());
        ACE_ASSERT(self);

        if (dynamic_cast<const TypeParamTypeSymbol*>(self))
        {
            return true;
        }

        const auto placeholderArgIt = std::find_if(
            begin(self->GetTypeArgs()),
            end  (self->GetTypeArgs()),
            [](ITypeSymbol* const arg) { return arg->IsPlaceholder(); }
        );
        if (placeholderArgIt != end(self->GetTypeArgs()))
        {
            return true;
        }

        auto* const prototype = dynamic_cast<const PrototypeSymbol*>(self);
        if (prototype && prototype->GetSelfType()->IsPlaceholder())
        {
            return true;
        }

        return false;
    }

    auto IGenericSymbol::GetGenericRoot() const -> IGenericSymbol*
    {
        return GenericInstantiator::GetGenericRoot(this);
    }

    auto IGenericSymbol::IsInstance() const -> bool
    {
        return GenericInstantiator::IsInstance(this);
    }
}
