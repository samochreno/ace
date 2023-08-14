#pragma once

#include <memory>

#include "Symbols/Vars/VarSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
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
            ISizedTypeSymbol* const type
        );
        virtual ~LocalVarSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Ident& final;
        auto GetKind() const -> SymbolKind final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> ISizedTypeSymbol* final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
        ISizedTypeSymbol* m_Type;
    };
}
