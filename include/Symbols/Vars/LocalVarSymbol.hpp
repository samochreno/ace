#pragma once

#include <memory>

#include "Symbols/Vars/VarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class LocalVarSymbol : public IVarSymbol
    {
    public:
        LocalVarSymbol(
            const std::shared_ptr<Scope>& scope,
            const Ident& name,
            ITypeSymbol* const type
        );
        virtual ~LocalVarSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Ident& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> ITypeSymbol* final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
        ITypeSymbol* m_Type;
    };
}
