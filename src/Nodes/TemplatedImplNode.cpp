#include "Nodes/TemplatedImplNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Nodes/FunctionNode.hpp"
#include "Nodes/Templates/FunctionTemplateNode.hpp"
#include "BoundNodes/ImplBoundNode.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/TemplatedImplSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Templates/TypeTemplateSymbol.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace
{
    TemplatedImplNode::TemplatedImplNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_selfScope,
        const SymbolName& t_typeTemplateName,
        const std::vector<std::shared_ptr<const FunctionNode>>& t_functions,
        const std::vector<std::shared_ptr<const FunctionTemplateNode>>& t_functionTemplates
    ) : m_SourceLocation{ t_sourceLocation },
        m_SelfScope{ t_selfScope },
        m_TypeTemplateName{ t_typeTemplateName },
        m_Functions{ t_functions },
        m_FunctionTemplates{ t_functionTemplates }
    {
    }

    auto TemplatedImplNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto TemplatedImplNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto TemplatedImplNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Functions);
        AddChildren(children, m_FunctionTemplates);

        return children;
    }

    auto TemplatedImplNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const TemplatedImplNode>
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

        return std::make_shared<const TemplatedImplNode>(
            m_SourceLocation,
            selfScope,
            m_TypeTemplateName,
            clonedFunctions,
            clonedFunctionTemplates
        );
    }

    auto TemplatedImplNode::CreateBound() const -> Expected<std::shared_ptr<const ImplBoundNode>>
    {
        ACE_TRY(boundFunctions, TransformExpectedVector(m_Functions,
        [](const std::shared_ptr<const FunctionNode>& t_function)
        {
            return t_function->CreateBound();
        }));

        return std::make_shared<const ImplBoundNode>(
            GetScope(),
            boundFunctions
        );
    }

    auto TemplatedImplNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto TemplatedImplNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TemplatedImpl;
    }

    auto TemplatedImplNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto TemplatedImplNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        ACE_TRY(templateSymbol, GetScope()->ResolveStaticSymbol<TypeTemplateSymbol>(m_TypeTemplateName));
        templateSymbol->GetSelfScope()->DefineAssociation(m_SelfScope);

        return std::unique_ptr<ISymbol>
        {
            std::make_unique<TemplatedImplSymbol>(
                GetScope(),
                m_SelfScope,
                templateSymbol
            )
        };
    }

    auto TemplatedImplNode::DefineAssociations() const -> Expected<void>
    { 
        ACE_TRY(templateSymbol, GetScope()->ResolveStaticSymbol<TypeTemplateSymbol>(m_TypeTemplateName));
        templateSymbol->GetSelfScope()->DefineAssociation(m_SelfScope);
        return Void{};
    }
}
