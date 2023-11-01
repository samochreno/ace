#include "Symbols/Vars/GlobalVarSymbol.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Ident.hpp"
#include "Noun.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    GlobalVarSymbol::GlobalVarSymbol(
        const std::shared_ptr<Scope>& scope,
        const AccessModifier accessModifier,
        const Ident& name,
        ISizedTypeSymbol* const type
    ) : m_Scope{ scope },
        m_Name{ name },
        m_AccessModifier{ accessModifier },
        m_Type{ type }
    {
    }

    auto GlobalVarSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "global variable" };
    }

    auto GlobalVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto GlobalVarSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto GlobalVarSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto GlobalVarSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto GlobalVarSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<GlobalVarSymbol>(
            scope,
            GetAccessModifier(),
            GetName(),
            CreateInstantiated<ISizedTypeSymbol>(GetType(), context)
        );
    }

    auto GlobalVarSymbol::GetSizedType() const -> ISizedTypeSymbol*
    {
        return m_Type;
    }
}
