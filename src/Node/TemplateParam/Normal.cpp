
#include "Node/TemplateParam/Normal.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Type/TemplateParam/Normal.hpp"

namespace Ace::Node::TemplateParam
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
    ) const -> std::shared_ptr<const Node::TemplateParam::Normal>
    {
        return std::make_shared<const Node::TemplateParam::Normal>(
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
        return SymbolKind::TemplateParam;
    }

    auto Normal::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto Normal::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Type::TemplateParam::Normal>(
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
