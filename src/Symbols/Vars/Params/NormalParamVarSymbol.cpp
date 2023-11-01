#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Ident.hpp"
#include "Noun.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    NormalParamVarSymbol::NormalParamVarSymbol(
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        ISizedTypeSymbol* const type,
        const size_t index
    ) : m_Scope{ scope },
        m_Name{ name },
        m_Type{ type },
        m_Index{ index }
    {
    }

    auto NormalParamVarSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "parameter" };
    }

    auto NormalParamVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalParamVarSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto NormalParamVarSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Pub;
    }

    auto NormalParamVarSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto NormalParamVarSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<NormalParamVarSymbol>(
            scope,
            GetName(),
            CreateInstantiated<ISizedTypeSymbol>(GetType(), context),
            GetIndex()
        );
    }

    auto NormalParamVarSymbol::GetSizedType() const -> ISizedTypeSymbol*
    {
        return m_Type;
    }

    auto NormalParamVarSymbol::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
