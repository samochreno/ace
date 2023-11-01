#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Noun.hpp"
#include "Ident.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "SpecialIdent.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    SelfParamVarSymbol::SelfParamVarSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ISizedTypeSymbol* const type
    ) : m_Scope{ scope },
        m_Type{ type },
        m_Name{ srcLocation, SpecialIdent::Self }
    {
    }

    auto SelfParamVarSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "self parameter" };
    }

    auto SelfParamVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SelfParamVarSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto SelfParamVarSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Pub;
    }

    auto SelfParamVarSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto SelfParamVarSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<SelfParamVarSymbol>(
            GetName().SrcLocation,
            scope,
            CreateInstantiated<ISizedTypeSymbol>(GetType(), context)
        );
    }

    auto SelfParamVarSymbol::GetSizedType() const -> ISizedTypeSymbol*
    {
        return m_Type;
    }
}
