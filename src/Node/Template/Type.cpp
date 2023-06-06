#include "Node/Template/Type.hpp"

#include <memory>
#include <vector>
#include <string>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Template/Type.hpp"
#include "Node/TemplateParameter/Normal.hpp"

namespace Ace::Node::Template
{
    auto Type::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Parameters);

        return children;
    }

    auto Type::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Template::Type>
    {
        std::vector<std::shared_ptr<const Node::TemplateParameter::Normal>> clonedParameters{};
        std::transform(
            begin(m_Parameters),
            end  (m_Parameters),
            back_inserter(clonedParameters),
            [&](const std::shared_ptr<const Node::TemplateParameter::Normal>& t_parameter)
            {
                return t_parameter->CloneInScope(m_AST->GetSelfScope());
            }
        );

        return std::make_shared<const Node::Template::Type>(
            clonedParameters,
            m_AST->CloneInScopeType(t_scope)
        );
    }

    auto Type::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Template::Type>(this)
        };
    }

    auto Type::CollectImplParameterNames() const -> std::vector<std::string>
    {
        return {};
    }

    auto Type::CollectParameterNames() const -> std::vector<std::string>
    {
        std::vector<std::string> names{};
        std::transform(
            begin(m_Parameters),
            end  (m_Parameters),
            back_inserter(names),
            [](const std::shared_ptr<const Node::TemplateParameter::Normal>& t_parameter)
            {
                return t_parameter->GetName();
            }
        );

        return names;
    }
}
