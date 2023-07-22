#pragma once

#include <memory>

#include "Symbols/Vars/VarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class InstanceVarSymbol : public virtual IVarSymbol
    {
    public:
        InstanceVarSymbol(
            const std::shared_ptr<Scope>& scope,
            const Ident& name,
            const AccessModifier accessModifier,
            ITypeSymbol* const type,
            const size_t index
        );
        virtual ~InstanceVarSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Ident& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> ITypeSymbol* final;

        auto GetIndex() const -> size_t;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
        AccessModifier m_AccessModifier{};
        ITypeSymbol* m_Type{};
        size_t m_Index{};
    };
}
