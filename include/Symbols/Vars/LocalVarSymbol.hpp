#pragma once

#include <memory>
#include <vector>

#include "Symbols/Vars/VarSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Noun.hpp"
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

        auto CreateTypeNoun() const -> Noun final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;
        auto GetName() const -> const Ident& final;

        auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope,
            const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> final;

        auto GetSizedType() const -> ISizedTypeSymbol* final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
        ISizedTypeSymbol* m_Type;
    };
}
