#pragma once

#include <memory>

#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class SelfParamVarSymbol : public virtual IParamVarSymbol
    {
    public:
        SelfParamVarSymbol(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<Scope>& t_scope,
            ITypeSymbol* const t_type
        );
        virtual ~SelfParamVarSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Identifier& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> ITypeSymbol* final;

    private:
        Identifier m_Name{};
        std::shared_ptr<Scope> m_Scope{};
        ITypeSymbol* m_Type{};
    };
}
