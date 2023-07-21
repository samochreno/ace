#include "Symbols/TemplatedImplSymbol.hpp"

#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    TemplatedImplSymbol::TemplatedImplSymbol(
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<Scope>& selfScope,
        TypeTemplateSymbol* const implementedTypeTemplate
    ) : m_Scope{ scope },
        m_SelfScope{ selfScope },
        m_Name
        {
            implementedTypeTemplate->GetName().SourceLocation,
            SpecialIdentifier::CreateAnonymous()
        },
        m_ImplementedTypeTemplate{ implementedTypeTemplate }
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
