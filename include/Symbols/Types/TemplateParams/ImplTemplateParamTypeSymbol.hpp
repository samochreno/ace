#pragma once

#include <memory>

#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class ImplTemplateParamTypeSymbol : public virtual ITypeSymbol
    {
    public:
        ImplTemplateParamTypeSymbol(
            const std::shared_ptr<Scope>& scope,
            const Ident& name
        );
        virtual ~ImplTemplateParamTypeSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Ident& final;
        auto GetKind() const -> SymbolKind final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        Ident m_Name{};
    };
}
