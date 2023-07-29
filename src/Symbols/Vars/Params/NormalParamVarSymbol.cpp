#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"

#include <memory>

#include "Scope.hpp"
#include "Ident.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    NormalParamVarSymbol::NormalParamVarSymbol(
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        ITypeSymbol* const type,
        const size_t index
    ) : m_Scope{ scope },
        m_Name{ name },
        m_Type{ type },
        m_Index{ index }
    {
    }

    auto NormalParamVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalParamVarSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto NormalParamVarSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::ParamVar;
    }

    auto NormalParamVarSymbol::GetCategory() const -> SymbolCategory
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
