#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"

#include <memory>
#include <string>

#include "Scope.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "SpecialIdentifier.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    SelfParamVarSymbol::SelfParamVarSymbol(
        const std::shared_ptr<Scope>& t_scope,
        ITypeSymbol* const t_type
    ) : m_Scope{ t_scope },
        m_Type{ t_type }
    {
    }

    auto SelfParamVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SelfParamVarSymbol::GetName() const -> const std::string&
    {
        static std::string name = SpecialIdentifier::Self;
        return name;
    }

    auto SelfParamVarSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::ParamVar;
    }

    auto SelfParamVarSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto SelfParamVarSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Public;
    }

    auto SelfParamVarSymbol::GetType() const -> ITypeSymbol*
    {
        return m_Type;
    }
}
