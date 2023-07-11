#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"

#include <memory>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "SpecialIdentifier.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    SelfParamVarSymbol::SelfParamVarSymbol(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        ITypeSymbol* const t_type
    ) : m_Scope{ t_scope },
        m_Type{ t_type },
        m_Name
        {
            t_sourceLocation,
            SpecialIdentifier::Self,
        }
    {
    }

    auto SelfParamVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SelfParamVarSymbol::GetName() const -> const Identifier&
    {
        return m_Name;
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
