
#include "Node/TemplateParameter/Normal.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Type/TemplateParameter/Normal.hpp"

namespace Ace::Node::TemplateParameter
{
    auto Normal::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto Normal::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::TemplateParameter::Normal>
    {
        return std::make_shared<const Node::TemplateParameter::Normal>(
            t_scope,
            m_Name
        );
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
}
