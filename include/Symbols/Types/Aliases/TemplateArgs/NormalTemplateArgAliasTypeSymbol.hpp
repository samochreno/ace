#pragma once

#include <memory>

#include "Symbols/Types/Aliases/TemplateArgs/TemplateArgAliasTypeSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class NormalTemplateArgAliasTypeSymbol :
        public virtual ITemplateArgAliasTypeSymbol
    {
    public:
        NormalTemplateArgAliasTypeSymbol(
            const std::shared_ptr<Scope>& scope,
            const Ident& name,
            ITypeSymbol* const aliasedType,
            const size_t index
        );
        virtual ~NormalTemplateArgAliasTypeSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Ident& final;
        auto GetKind() const -> SymbolKind final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetAliasedType() const -> ITypeSymbol* final;

        auto GetIndex() const -> size_t final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
        ITypeSymbol* m_AliasedType{};
        size_t m_Index{};
    };
}
