#include "Symbols/Vars/FieldVarSymbol.hpp"

#include <memory>
#include <vector>

#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Noun.hpp"
#include "Scope.hpp"

namespace Ace
{
    FieldVarSymbol::FieldVarSymbol(
        StructTypeSymbol* const parentStruct,
        const AccessModifier accessModifier,
        const Ident& name,
        ISizedTypeSymbol* const type,
        const size_t index
    ) : m_ParentStruct{ parentStruct },
        m_AccessModifier{ accessModifier },
        m_Name{ name },
        m_Type{ type },
        m_Index{ index }
    {
    }

    auto FieldVarSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "field" };
    }

    auto FieldVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return GetParentStruct()->GetBodyScope();
    }

    auto FieldVarSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Instance;
    }

    auto FieldVarSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto FieldVarSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto FieldVarSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<FieldVarSymbol>(
            CreateInstantiated<StructTypeSymbol>(GetParentStruct(), context),
            GetAccessModifier(),
            GetName(),
            CreateInstantiated<ISizedTypeSymbol>(GetType(), context),
            GetIndex()
        );
    }

    auto FieldVarSymbol::GetSizedType() const -> ISizedTypeSymbol*
    {
        return m_Type;
    }

    auto FieldVarSymbol::GetParentStruct() const -> StructTypeSymbol*
    {
        return m_ParentStruct;
    }

    auto FieldVarSymbol::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
