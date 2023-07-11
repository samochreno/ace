#pragma once

#include <memory>

#include "Symbols/Vars/VarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class InstanceVarSymbol : public virtual IVarSymbol
    {
    public:
        InstanceVarSymbol(
            const std::shared_ptr<Scope>& t_scope,
            const Identifier& t_name,
            const AccessModifier t_accessModifier,
            ITypeSymbol* const t_type,
            const size_t t_index
        );
        virtual ~InstanceVarSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Identifier& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> ITypeSymbol* final;

        auto GetIndex() const -> size_t;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Identifier m_Name{};
        AccessModifier m_AccessModifier{};
        ITypeSymbol* m_Type{};
        size_t m_Index{};
    };
}
