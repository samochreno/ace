#include "Symbols/TemplatedImplSymbol.hpp"

#include "Scope.hpp"
#include "Ident.hpp"
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
            implementedTypeTemplate->GetName().SrcLocation,
            SpecialIdent::CreateAnonymous()
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

    auto TemplatedImplSymbol::GetName() const -> const Ident&
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
