#pragma once

#include <memory>
#include <vector>

#include "Symbols/Vars/VarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "AccessModifier.hpp"
#include "Ident.hpp"
#include "Noun.hpp"
#include "Scope.hpp"

namespace Ace
{
    class StructTypeSymbol;

    class FieldVarSymbol : public virtual IVarSymbol
    {
    public:
        FieldVarSymbol(
            StructTypeSymbol* const parentStruct,
            const AccessModifier accessModifier,
            const Ident& name,
            ISizedTypeSymbol* const type,
            const size_t index
        );
        virtual ~FieldVarSymbol() = default;

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

        auto GetParentStruct() const -> StructTypeSymbol*;
        auto GetIndex() const -> size_t;

    private:
        StructTypeSymbol* m_ParentStruct{};
        AccessModifier m_AccessModifier{};
        Ident m_Name{};
        ISizedTypeSymbol* m_Type{};
        size_t m_Index{};
    };
}
