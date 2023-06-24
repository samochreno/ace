#include "Symbol/TemplatedImpl.hpp"

#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol
{
    TemplatedImpl::TemplatedImpl(
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<Scope>& t_selfScope,
        Symbol::Template::Type* const t_implementedTypeTemplate
    ) : m_Scope{ t_scope },
        m_SelfScope{ t_selfScope },
        m_Name{ SpecialIdentifier::CreateAnonymous() },
        m_ImplementedTypeTemplate{ t_implementedTypeTemplate }
    {
    }

    auto TemplatedImpl::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto TemplatedImpl::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto TemplatedImpl::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto TemplatedImpl::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TemplatedImpl;
    }

    auto TemplatedImpl::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto TemplatedImpl::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Public;
    }

    auto TemplatedImpl::GetImplementedTypeTemplate() const -> Symbol::Template::Type*
    {
        return m_ImplementedTypeTemplate;
    }
}
