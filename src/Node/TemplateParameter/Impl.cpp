#include "Node/TemplateParameter/Impl.hpp"

#include <memory>
#include <vector>

#include "Error.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Type/TemplateParameter/Impl.hpp"

namespace Ace::Node::TemplateParameter
{
    auto Impl::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto Impl::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::TemplateParameter::Impl>
    {
        return std::make_shared<const Node::TemplateParameter::Impl>(
            t_scope,
            m_Name
        );
    }

    auto Impl::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Type::TemplateParameter::Impl>(
                m_Scope,
                m_Name
            )
        };
    }
}
