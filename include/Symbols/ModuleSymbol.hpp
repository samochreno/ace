#pragma once

#include <memory>

#include "Symbols/Symbol.hpp"
#include "Symbols/SelfScopedSymbol.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class ModuleSymbol :
        public virtual ISymbol,
        public virtual ISelfScopedSymbol
    {
    public:
        ModuleSymbol(
            const std::shared_ptr<Scope>& selfScope,
            const Identifier& name,
            const AccessModifier accessModifier
        );
        virtual ~ModuleSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Identifier& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        Identifier m_Name{};
        AccessModifier m_AccessModifier{};
    };
}
