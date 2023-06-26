#pragma once

#include <memory>
#include <string>

#include "Symbols/Symbol.hpp"
#include "Symbols/SelfScopedSymbol.hpp"
#include "Symbols/Templates/TypeTemplateSymbol.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class TemplatedImplSymbol : 
        public virtual ISymbol, 
        public virtual ISelfScopedSymbol
    {
    public:
        TemplatedImplSymbol(
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<Scope>& t_selfScope,
            TypeTemplateSymbol* const t_implementedTypeTemplate
        );
        virtual ~TemplatedImplSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetImplementedTypeTemplate() const -> TypeTemplateSymbol*;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<Scope> m_SelfScope{};
        std::string m_Name{};
        TypeTemplateSymbol* m_ImplementedTypeTemplate{};
    };
}
