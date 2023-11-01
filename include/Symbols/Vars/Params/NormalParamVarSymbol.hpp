#pragma once

#include <memory>
#include <vector>

#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Noun.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class NormalParamVarSymbol : public virtual IParamVarSymbol
    {
    public:
        NormalParamVarSymbol(
            const std::shared_ptr<Scope>& scope,
            const Ident& name,
            ISizedTypeSymbol* const type,
            const size_t index
        );
        virtual ~NormalParamVarSymbol() = default;

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

        auto GetIndex() const -> size_t;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
        ISizedTypeSymbol* m_Type{};
        size_t m_Index{};
    };
}
