#include "Node/Template/Function.hpp"

#include <memory>
#include <vector>
#include <string>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Template/Function.hpp"
#include "Node/TemplateParameter/Impl.hpp"
#include "Node/TemplateParameter/Normal.hpp"

namespace Ace::Node::Template
{
    auto Function::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};
        
        AddChildren(children, m_ImplParameters);
        AddChildren(children, m_Parameters);

        return children;
    }

    auto Function::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Template::Function>
    {
        std::vector<std::shared_ptr<const Node::TemplateParameter::Impl>> clonedImplParameters{};
        std::transform(
            begin(m_ImplParameters),
            end  (m_ImplParameters),
            back_inserter(clonedImplParameters),
            [&](const std::shared_ptr<const Node::TemplateParameter::Impl>& t_implParameter)
            {
                return t_implParameter->CloneInScope(m_AST->GetSelfScope());
            }
        );

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

        return std::make_shared<const Node::Template::Function>(
            clonedImplParameters,
            clonedParameters,
            m_AST->CloneInScope(t_scope)
        );
    }

    auto Function::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Template::Function>(this)
        };
    }

    auto Function::CollectImplParameterNames() const -> std::vector<std::string>
    {
        std::vector<std::string> names{};
        std::transform(
            begin(m_ImplParameters),
            end  (m_ImplParameters),
            back_inserter(names),
            [](const std::shared_ptr<const Node::TemplateParameter::Impl>& t_implParameter)
            {
                return t_implParameter->GetName();
            }
        );

        return names;
    }

    auto Function::CollectParameterNames() const -> std::vector<std::string>
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
