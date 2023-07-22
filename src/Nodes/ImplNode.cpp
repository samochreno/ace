#include "Nodes/ImplNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Nodes/FunctionNode.hpp"
#include "Nodes/Templates/FunctionTemplateNode.hpp"
#include "BoundNodes/ImplBoundNode.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Templates/TypeTemplateSymbol.hpp"
#include "SpecialIdent.hpp"

namespace Ace
{
    ImplNode::ImplNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& selfScope,
        const SymbolName& typeName,
        const std::vector<std::shared_ptr<const FunctionNode>>& functions,
        const std::vector<std::shared_ptr<const FunctionTemplateNode>>& functionTemplates
    ) : m_SelfScope{ selfScope },
        m_TypeName{ typeName },
        m_Functions{ functions },
        m_FunctionTemplates{ functionTemplates }
    {
    }

    auto ImplNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const ImplNode>
    {
        const auto selfScope = scope->GetOrCreateChild({});

        std::vector<std::shared_ptr<const FunctionNode>> clonedFunctions{};
        std::transform(
            begin(m_Functions),
            end  (m_Functions),
            back_inserter(clonedFunctions),
            [&](const std::shared_ptr<const FunctionNode>& function)
            {
                return function->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const FunctionTemplateNode>> clonedFunctionTemplates{};
        std::transform(
            begin(m_FunctionTemplates),
            end  (m_FunctionTemplates),
            back_inserter(clonedFunctionTemplates),
            [&](const std::shared_ptr<const FunctionTemplateNode>& functionTemplate)
            {
                return functionTemplate->CloneInScope(selfScope);
            }
        );

        return std::make_shared<const ImplNode>(
            m_SrcLocation,
            selfScope,
            m_TypeName,
            clonedFunctions,
            clonedFunctionTemplates
        );
    }

    auto ImplNode::CreateBound() const -> Expected<std::shared_ptr<const ImplBoundNode>>
    {
        ACE_TRY(boundFunctions, TransformExpectedVector(m_Functions,
        [](const std::shared_ptr<const FunctionNode>& function)
        {
            return function->CreateBound();
        }));

        return std::make_shared<const ImplBoundNode>(
            GetSrcLocation(),
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
                lastNameSection.Name.String = SpecialIdent::CreateTemplate(
                    lastNameSection.Name.String
                );

                ACE_TRY(templateSymbol, GetScope()->ResolveStaticSymbol<TypeTemplateSymbol>(typeName));
                return templateSymbol->GetSelfScope();
            }
        }());

        scope->DefineAssociation(m_SelfScope);
        return Void{};
    }
}
