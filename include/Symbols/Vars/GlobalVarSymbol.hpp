#pragma once

#include <memory>
#include <vector>

#include "Symbols/Vars/VarSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Ident.hpp"
#include "Noun.hpp"

namespace Ace
{
    class GlobalVarSymbol : public virtual IVarSymbol
    {
    public:
        GlobalVarSymbol(
            const std::shared_ptr<Scope>& scope,
            const AccessModifier accessModifier,
            const Ident& name,
            ISizedTypeSymbol* const type
        );
        virtual ~GlobalVarSymbol() = default;

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
        AccessModifier m_AccessModifier{};
        Ident m_Name{};
        ISizedTypeSymbol* m_Type{};
    };
}
