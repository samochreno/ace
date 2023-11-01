#include "Symbols/Vars/LocalVarSymbol.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Ident.hpp"
#include "Noun.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    LocalVarSymbol::LocalVarSymbol(
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        ISizedTypeSymbol* const type
    ) : m_Scope{ scope },
        m_Name{ name },
        m_Type{ type }
    {
    }

    auto LocalVarSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "local variable" };
    }

    auto LocalVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LocalVarSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto LocalVarSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Priv;
    }

    auto LocalVarSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto LocalVarSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<LocalVarSymbol>(
            scope,
            GetName(),
            CreateInstantiated<ISizedTypeSymbol>(GetType(), context)
        );
    }

    auto LocalVarSymbol::GetSizedType() const -> ISizedTypeSymbol*
    {
        return m_Type;
    }
}
