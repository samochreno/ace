#pragma once

#include <memory>
#include <vector>

#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/PrototypeSymbol.hpp"
#include "Symbols/Types/TraitSelfTypeSymbol.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Ident.hpp"
#include "Noun.hpp"

namespace Ace
{
    class TraitTypeSymbol : public virtual ITypeSymbol
    {
    public:
        TraitTypeSymbol(
            const std::shared_ptr<Scope>& bodyScope,
            const std::shared_ptr<Scope>& prototypeScope,
            const AccessModifier accessModifier,
            const Ident& name,
            const std::vector<ITypeSymbol*>& typeArgs
        );
        virtual ~TraitTypeSymbol() = default;

        auto CreateTypeNoun() const -> Noun final;
        auto GetBodyScope() const -> std::shared_ptr<Scope> final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;
        auto GetName() const -> const Ident& final;

        auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope,
            const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> final;

        auto SetBodyScope(const std::shared_ptr<Scope>& scope) -> void final;
        auto GetTypeArgs() const -> const std::vector<ITypeSymbol*>& final;

        auto GetPrototypeScope() const -> const std::shared_ptr<Scope>&;
        auto CollectPrototypes() const -> std::vector<PrototypeSymbol*>;
        auto CollectSelf() const -> TraitSelfTypeSymbol*;

    private:
        std::shared_ptr<Scope> m_BodyScope{};
        std::shared_ptr<Scope> m_PrototypeScope{};
        AccessModifier m_AccessModifier{};
        Ident m_Name{};
        std::vector<ITypeSymbol*> m_TypeArgs{};
    };
}
