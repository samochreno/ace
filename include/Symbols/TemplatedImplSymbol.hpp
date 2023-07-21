#pragma once

#include <memory>

#include "Symbols/Symbol.hpp"
#include "Symbols/SelfScopedSymbol.hpp"
#include "Symbols/Templates/TypeTemplateSymbol.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class TemplatedImplSymbol : 
        public virtual ISymbol, 
        public virtual ISelfScopedSymbol
    {
    public:
        TemplatedImplSymbol(
            const std::shared_ptr<Scope>& scope,
            const std::shared_ptr<Scope>& selfScope,
            TypeTemplateSymbol* const implementedTypeTemplate
        );
        virtual ~TemplatedImplSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Identifier& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetImplementedTypeTemplate() const -> TypeTemplateSymbol*;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<Scope> m_SelfScope{};
        Identifier m_Name{};
        TypeTemplateSymbol* m_ImplementedTypeTemplate{};
    };
}
