#pragma once

#include <memory>

#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Noun.hpp"
#include "Ident.hpp"

namespace Ace
{
    class SupertraitSymbol : public virtual ISymbol
    {
    public:
        SupertraitSymbol(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            TraitTypeSymbol* const trait
        );
        virtual ~SupertraitSymbol() = default;

        auto CreateTypeNoun() const -> Noun final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;
        auto GetName() const -> const Ident& final;

        auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope,
            const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> final;

        auto GetTrait() const -> TraitTypeSymbol*;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
        TraitTypeSymbol* m_Trait{};
    };
}
