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
            ISizedTypeSymbol* const type,
            const size_t index
        );
        virtual ~InstanceVarSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Ident& final;
        auto GetKind() const -> SymbolKind final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> ISizedTypeSymbol* final;

        auto GetIndex() const -> size_t;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
        AccessModifier m_AccessModifier{};
        ISizedTypeSymbol* m_Type{};
        size_t m_Index{};
    };
}
