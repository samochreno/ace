#pragma once

#include <memory>

#include "Symbols/Vars/VarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class StaticVarSymbol : public virtual IVarSymbol
    {
    public:
        StaticVarSymbol(
            const std::shared_ptr<Scope>& t_scope,
            const Identifier& t_name,
            const AccessModifier t_accessModifier,
            ITypeSymbol* const t_type
        );
        virtual ~StaticVarSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Identifier& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> ITypeSymbol* final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Identifier m_Name{};
        AccessModifier m_AccessModifier{};
        ITypeSymbol* m_Type{};
    };
}
