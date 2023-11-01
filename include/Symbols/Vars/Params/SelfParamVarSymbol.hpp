#pragma once

#include <memory>
#include <vector>

#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Noun.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class SelfParamVarSymbol : public virtual IParamVarSymbol
    {
    public:
        SelfParamVarSymbol(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            ISizedTypeSymbol* const type
        );
        virtual ~SelfParamVarSymbol() = default;

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
        Ident m_Name{};
        std::shared_ptr<Scope> m_Scope{};
        ISizedTypeSymbol* m_Type{};
    };
}
