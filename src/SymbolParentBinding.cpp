#include "SymbolParentBinding.hpp"

#include "Assert.hpp"
#include "Scope.hpp"
#include "Symbols/All.hpp"

namespace Ace
{
    static auto BindChildren(ISymbol* const parent) -> void
    {
        if (auto* const typeParamOwner =
            dynamic_cast<ITypeParamOwnerSymbol*>(parent))
        {
            const auto typeParams = typeParamOwner->CollectOwnedTypeParams();
            std::for_each(begin(typeParams), end(typeParams),
            [&](TypeParamTypeSymbol* const typeParam)
            {
                typeParam->BindParent(typeParamOwner);
            });
        }

        if (auto* const callable = dynamic_cast<ICallableSymbol*>(parent))
        {
            const auto params = callable->CollectAllParams();
            std::for_each(begin(params), end(params),
            [&](IParamVarSymbol* const param)
            {
                param->BindParent(callable);
            });
        }
    }

    static auto BindChildrenInScope(
        const std::shared_ptr<Scope>& scope
    ) -> void
    {
        const auto symbols = scope->CollectAllSymbols();
        std::for_each(begin(symbols), end(symbols), BindChildren);

        const auto childScopes = scope->CollectChildren();
        std::for_each(begin(childScopes), end(childScopes),
        [&](const std::shared_ptr<Scope>& childScope)
        {
            BindChildrenInScope(childScope);
        });
    }

    static auto ValidateChildrenInScope(
        const std::shared_ptr<Scope>& scope
    ) -> void
    {
        const auto symbols = scope->CollectAllSymbols();
        std::for_each(begin(symbols), end(symbols),
        [&](ISymbol* const symbol)
        {
            if (auto* const typeParam =
                dynamic_cast<TypeParamTypeSymbol*>(symbol))
            {
                ACE_ASSERT(
                    typeParam->GetParent()->GetBodyScope() == scope
                );
            }

            if (auto* const param = dynamic_cast<IParamVarSymbol*>(symbol))
            {
                ACE_ASSERT(param->GetParent()->GetBodyScope() == scope);
            }
        });

        const auto childScopes = scope->CollectChildren();
        std::for_each(begin(childScopes), end(childScopes),
        [&](const std::shared_ptr<Scope>& childScope)
        {
            ValidateChildrenInScope(childScope);
        });
    }

    auto BindSymbolParents(const std::shared_ptr<Scope>& scope) -> void
    {
        BindChildrenInScope(scope);
        ValidateChildrenInScope(scope);
    }

    auto BindSymbolParents(ISymbol* const parent) -> void
    {
        ACE_ASSERT(parent);
        BindChildren(parent);

        auto* const bodyScoped = dynamic_cast<IBodyScopedSymbol*>(parent);
        if (!bodyScoped)
        {
            return;
        }

        BindSymbolParents(bodyScoped->GetBodyScope());
    }
}
