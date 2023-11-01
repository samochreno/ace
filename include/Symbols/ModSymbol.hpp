#pragma once

#include <memory>
#include <vector>

#include "Symbols/Symbol.hpp"
#include "Symbols/AccessibleBodyScopedSymbol.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Ident.hpp"
#include "Noun.hpp"

namespace Ace
{
    class ModSymbol :
        public virtual ISymbol,
        public virtual IAccessibleBodyScopedSymbol
    {
    public:
        ModSymbol(
            const std::shared_ptr<Scope>& bodyScope,
            const AccessModifier accessModifier,
            const Ident& name
        );
        virtual ~ModSymbol() = default;

        auto CreateTypeNoun() const -> Noun final;
        auto GetBodyScope() const -> std::shared_ptr<Scope> final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;
        auto GetName() const -> const Ident& final;

        auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope,
            const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> final;

    private:
        std::shared_ptr<Scope> m_BodyScope{};
        AccessModifier m_AccessModifier{};
        Ident m_Name{};
    };
}
