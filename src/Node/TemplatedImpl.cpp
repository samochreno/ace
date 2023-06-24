#include "Node/TemplatedImpl.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Function.hpp"
#include "Node/Template/Function.hpp"
#include "BoundNode/Impl.hpp"
#include "Diagnostics.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/TemplatedImpl.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Template/Type.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace::Node
{
    TemplatedImpl::TemplatedImpl(
        const std::shared_ptr<Scope>& t_selfScope,
        const SymbolName& t_typeTemplateName,
        const std::vector<std::shared_ptr<const Node::Function>>& t_functions,
        const std::vector<std::shared_ptr<const Node::Template::Function>>& t_functionTemplates
    ) : m_SelfScope{ t_selfScope },
        m_TypeTemplateName{ t_typeTemplateName },
        m_Functions{ t_functions },
        m_FunctionTemplates{ t_functionTemplates }
    {
    }

    auto TemplatedImpl::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto TemplatedImpl::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Functions);
        AddChildren(children, m_FunctionTemplates);

        return children;
    }

    auto TemplatedImpl::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::TemplatedImpl>
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

        return std::make_shared<const Node::TemplatedImpl>(
            selfScope,
            m_TypeTemplateName,
            clonedFunctions,
            clonedFunctionTemplates
        );
    }

    auto TemplatedImpl::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Impl>>
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

    auto TemplatedImpl::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto TemplatedImpl::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TemplatedImpl;
    }

    auto TemplatedImpl::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto TemplatedImpl::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        ACE_TRY(templateSymbol, GetScope()->ResolveStaticSymbol<Symbol::Template::Type>(m_TypeTemplateName));
        templateSymbol->GetSelfScope()->DefineAssociation(m_SelfScope);

        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::TemplatedImpl>(
                GetScope(),
                m_SelfScope,
                templateSymbol
            )
        };
    }

    auto TemplatedImpl::DefineAssociations() const -> Expected<void>
    { 
        ACE_TRY(templateSymbol, GetScope()->ResolveStaticSymbol<Symbol::Template::Type>(m_TypeTemplateName));
        templateSymbol->GetSelfScope()->DefineAssociation(m_SelfScope);
        return Void;
    }
}
