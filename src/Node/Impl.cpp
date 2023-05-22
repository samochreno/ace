#include "Node/Impl.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Function.hpp"
#include "Node/Template/Function.hpp"
#include "BoundNode/Impl.hpp"
#include "Error.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Template/Type.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace::Node
{
    auto Impl::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Functions);
        AddChildren(children, m_FunctionTemplates);

        return children;
    }

    auto Impl::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Impl>
    {
        const auto selfScope = t_scope->GetOrCreateChild({});

        std::vector<std::shared_ptr<const Node::Function>> clonedFunctions{};
        std::transform(
            begin(m_Functions),
            end  (m_Functions),
            back_inserter(clonedFunctions),
            [&](const std::shared_ptr<const Node::Function>& t_function)
            {
                return t_function->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const Node::Template::Function>> clonedFunctionTemplates{};
        std::transform(
            begin(m_FunctionTemplates),
            end  (m_FunctionTemplates),
            back_inserter(clonedFunctionTemplates),
            [&](const std::shared_ptr<const Node::Template::Function>& t_functionTemplate)
            {
                return t_functionTemplate->CloneInScope(selfScope);
            }
        );

        return std::make_shared<const Node::Impl>(
            selfScope,
            m_TypeName,
            clonedFunctions,
            clonedFunctionTemplates
        );
    }

    auto Impl::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Impl>>
    {
        ACE_TRY(boundFunctions, TransformExpectedVector(m_Functions,
        [](const std::shared_ptr<const Node::Function>& t_function)
        {
            return t_function->CreateBound();
        }));

        return std::make_shared<const BoundNode::Impl>(
            GetScope(),
            boundFunctions
        );
    }

    auto Impl::DefineAssociations() const -> Expected<void>
    {
        ACE_TRY(scope, [&]() -> Expected<std::shared_ptr<Scope>>
        {
            if (m_TypeName.Sections.back().TemplateArguments.empty())
            {
                ACE_TRY(typeSymbol, GetScope()->ResolveStaticSymbol<Symbol::Type::IBase>(m_TypeName));
                return typeSymbol->GetSelfScope();
            }
            else
            {
                auto typeName = m_TypeName;

                auto& lastNameSection = typeName.Sections.back();
                lastNameSection.TemplateArguments.clear();
                lastNameSection.Name = SpecialIdentifier::CreateTemplate(lastNameSection.Name);

                ACE_TRY(templateSymbol, GetScope()->ResolveStaticSymbol<Symbol::Template::Type>(typeName));
                return templateSymbol->GetSelfScope();
            }
        }());

        scope->DefineAssociation(m_SelfScope);
        return ExpectedVoid;
    }
}
