#pragma once

#include <memory>
#include <string>

#include "Symbols/Vars/VarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class StaticVarSymbol : public virtual IVarSymbol
    {
    public:
        StaticVarSymbol(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name,
            const AccessModifier t_accessModifier,
            ITypeSymbol* const t_type
        );
        virtual ~StaticVarSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> ITypeSymbol* final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        AccessModifier m_AccessModifier{};
        ITypeSymbol* m_Type{};
    };
}
