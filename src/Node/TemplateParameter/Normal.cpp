
#include "Node/TemplateParameter/Normal.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Type/TemplateParameter/Normal.hpp"

namespace Ace::Node::TemplateParameter
{
    Normal::Normal(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name
    ) : m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto Normal::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Normal::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto Normal::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::TemplateParameter::Normal>
    {
        return std::make_shared<const Node::TemplateParameter::Normal>(
            t_scope,
            m_Name
        );
    }

    auto Normal::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Normal::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TemplateParameter;
    }

    auto Normal::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto Normal::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Type::TemplateParameter::Normal>(
                m_Scope,
                m_Name
            )
        };
    }

    auto Normal::GetName() const -> const std::string&
    {
        return m_Name;
    }
}
