#include "Nodes/ImplNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Nodes/FunctionNode.hpp"
#include "Nodes/Templates/FunctionTemplateNode.hpp"
#include "BoundNodes/ImplBoundNode.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Templates/TypeTemplateSymbol.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace
{
    ImplNode::ImplNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_selfScope,
        const SymbolName& t_typeName,
        const std::vector<std::shared_ptr<const FunctionNode>>& t_functions,
        const std::vector<std::shared_ptr<const FunctionTemplateNode>>& t_functionTemplates
    ) : m_SelfScope{ t_selfScope },
        m_TypeName{ t_typeName },
        m_Functions{ t_functions },
        m_FunctionTemplates{ t_functionTemplates }
    {
    }

    auto ImplNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto ImplNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto ImplNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Functions);
        AddChildren(children, m_FunctionTemplates);

        return children;
    }

    auto ImplNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const ImplNode>
    {
        const auto selfScope = t_scope->GetOrCreateChild({});

        std::vector<std::shared_ptr<const FunctionNode>> clonedFunctions{};
        std::transform(
            begin(m_Functions),
            end  (m_Functions),
            back_inserter(clonedFunctions),
            [&](const std::shared_ptr<const FunctionNode>& t_function)
            {
                return t_function->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const FunctionTemplateNode>> clonedFunctionTemplates{};
        std::transform(
            begin(m_FunctionTemplates),
            end  (m_FunctionTemplates),
            back_inserter(clonedFunctionTemplates),
            [&](const std::shared_ptr<const FunctionTemplateNode>& t_functionTemplate)
            {
                return t_functionTemplate->CloneInScope(selfScope);
            }
        );

        return std::make_shared<const ImplNode>(
            m_SourceLocation,
            selfScope,
            m_TypeName,
            clonedFunctions,
            clonedFunctionTemplates
        );
    }

    auto ImplNode::CreateBound() const -> Expected<std::shared_ptr<const ImplBoundNode>>
    {
        ACE_TRY(boundFunctions, TransformExpectedVector(m_Functions,
        [](const std::shared_ptr<const FunctionNode>& t_function)
        {
            return t_function->CreateBound();
        }));

        return std::make_shared<const ImplBoundNode>(
            GetSourceLocation(),
            GetScope(),
            boundFunctions
        );
    }

    auto ImplNode::DefineAssociations() const -> Expected<void>
    {
        ACE_TRY(scope, [&]() -> Expected<std::shared_ptr<Scope>>
        {
            if (m_TypeName.Sections.back().TemplateArgs.empty())
            {
                ACE_TRY(typeSymbol, GetScope()->ResolveStaticSymbol<ITypeSymbol>(m_TypeName));
                return typeSymbol->GetSelfScope();
            }
            else
            {
                auto typeName = m_TypeName;

                auto& lastNameSection = typeName.Sections.back();
                lastNameSection.TemplateArgs.clear();
                lastNameSection.Name = SpecialIdentifier::CreateTemplate(
                    lastNameSection.Name
                );

                ACE_TRY(templateSymbol, GetScope()->ResolveStaticSymbol<TypeTemplateSymbol>(typeName));
                return templateSymbol->GetSelfScope();
            }
        }());

        scope->DefineAssociation(m_SelfScope);
        return Void{};
    }
}
