#include "Symbols/TemplatedImplSymbol.hpp"

#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    TemplatedImplSymbol::TemplatedImplSymbol(
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<Scope>& t_selfScope,
        TypeTemplateSymbol* const t_implementedTypeTemplate
    ) : m_Scope{ t_scope },
        m_SelfScope{ t_selfScope },
        m_Name
        {
            t_implementedTypeTemplate->GetName().SourceLocation,
            SpecialIdentifier::CreateAnonymous()
        },
        m_ImplementedTypeTemplate{ t_implementedTypeTemplate }
    {
    }

    auto TemplatedImplSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto TemplatedImplSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto TemplatedImplSymbol::GetName() const -> const Identifier&
    {
        return m_Name;
    }

    auto TemplatedImplSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TemplatedImpl;
    }

    auto TemplatedImplSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto TemplatedImplSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Public;
    }

    auto TemplatedImplSymbol::GetImplementedTypeTemplate() const -> TypeTemplateSymbol*
    {
        return m_ImplementedTypeTemplate;
    }
}
