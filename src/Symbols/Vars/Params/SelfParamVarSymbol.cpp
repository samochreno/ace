#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"

#include <memory>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
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
        m_Name
        {
            srcLocation,
            SpecialIdent::Self,
        }
    {
    }

    auto SelfParamVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SelfParamVarSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto SelfParamVarSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::ParamVar;
    }

    auto SelfParamVarSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto SelfParamVarSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Public;
    }

    auto SelfParamVarSymbol::GetType() const -> ISizedTypeSymbol*
    {
        return m_Type;
    }
}
