#include "Node/TemplateParam/Impl.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"

namespace Ace::Node::TemplateParam
{
    Impl::Impl(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name
    ) : m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto Impl::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Impl::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto Impl::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::TemplateParam::Impl>
    {
        return std::make_shared<const Node::TemplateParam::Impl>(
            t_scope,
            m_Name
        );
    }

    auto Impl::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Impl::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::ImplTemplateParam;
    }

    auto Impl::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto Impl::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        return std::unique_ptr<ISymbol>
        {
            std::make_unique<ImplTemplateParamTypeSymbol>(
                m_Scope,
                m_Name
            )
        };
    }

    auto Impl::GetName() const -> const std::string&
    {
        return m_Name;
    }
}
