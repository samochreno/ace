#pragma once

#include <memory>
#include <vector>

#include "Symbols/Types/Aliases/AliasTypeSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Scope.hpp"
#include "Noun.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class ReimportAliasTypeSymbol :
        public virtual IAliasTypeSymbol,
        public virtual ISizedTypeSymbol
    {
    public:
        ReimportAliasTypeSymbol(
            const std::shared_ptr<Scope>& scope,
            ITypeSymbol* const type
        );
        virtual ~ReimportAliasTypeSymbol() = default;

        auto CreateTypeNoun() const -> Noun final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetBodyScope() const -> std::shared_ptr<Scope> final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;
        auto GetName() const -> const Ident& final;

        auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope,
            const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> final;

        auto GetAliasedType() const -> ITypeSymbol* final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        ITypeSymbol* m_Type{};
    };
}
