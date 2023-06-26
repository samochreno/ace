#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"

#include <memory>
#include <string>

#include "Scope.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    NormalParamVarSymbol::NormalParamVarSymbol(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        ITypeSymbol* const t_type,
        const size_t& t_index
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_Type{ t_type },
        m_Index{ t_index }
    {
    }

    auto NormalParamVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalParamVarSymbol::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto NormalParamVarSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::ParamVar;
    }

    auto NormalParamVarSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto NormalParamVarSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Public;
    }

    auto NormalParamVarSymbol::GetType() const -> ITypeSymbol*
    {
        return m_Type;
    }

    auto NormalParamVarSymbol::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
