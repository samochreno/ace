#include "Nodes/Impls/TemplatedImplNode.hpp"

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
    TemplatedImplNode::TemplatedImplNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& selfScope,
        const SymbolName& typeTemplateName,
        const std::vector<std::shared_ptr<const FunctionNode>>& functions,
        const std::vector<std::shared_ptr<const FunctionTemplateNode>>& functionTemplates
    ) : m_SrcLocation{ srcLocation },
        m_SelfScope{ selfScope },
        m_TypeTemplateName{ typeTemplateName },
        m_Functions{ functions },
        m_FunctionTemplates{ functionTemplates }
    {
    }

    auto TemplatedImplNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto TemplatedImplNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto TemplatedImplNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Functions);
        AddChildren(children, m_FunctionTemplates);

        return children;
    }

    auto TemplatedImplNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const TemplatedImplNode>
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

        return std::make_shared<const TemplatedImplNode>(
            m_SrcLocation,
            selfScope,
            m_TypeTemplateName,
            clonedFunctions,
            clonedFunctionTemplates
        );
    }

    auto TemplatedImplNode::CreateBound() const -> Diagnosed<std::shared_ptr<const ImplBoundNode>>
    {
        DiagnosticBag diagnostics{};

        std::vector<std::shared_ptr<const FunctionBoundNode>> boundFunctions{};
        std::transform(
            begin(m_Functions),
            end  (m_Functions),
            back_inserter(boundFunctions),
            [&](const std::shared_ptr<const FunctionNode>& function)
            {
                return diagnostics.Collect(function->CreateBound());
            }
        );

        return Diagnosed
        {
            std::make_shared<const ImplBoundNode>(
                GetSrcLocation(),
                GetScope(),
                boundFunctions
            ),
            diagnostics,
        };
    }

    auto TemplatedImplNode::DefineAssociations() const -> Expected<void>
    { 
        DiagnosticBag diagnostics{};

        const auto optTemplateSymbol = diagnostics.Collect(GetScope()->ResolveStaticSymbol<TypeTemplateSymbol>(
            m_TypeTemplateName
        ));
        if (!optTemplateSymbol.has_value())
        {
            return diagnostics;
        }

        optTemplateSymbol.value()->GetSelfScope()->DefineAssociation(
            m_SelfScope
        );
        return Void{ diagnostics };
    }
}
