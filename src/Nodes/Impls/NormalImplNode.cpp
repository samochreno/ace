#include "Nodes/Impls/NormalImplNode.hpp"

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
    NormalImplNode::NormalImplNode(
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

    auto NormalImplNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto NormalImplNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto NormalImplNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Functions);
        AddChildren(children, m_FunctionTemplates);

        return children;
    }

    auto NormalImplNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const NormalImplNode>
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

        return std::make_shared<const NormalImplNode>(
            m_SrcLocation,
            selfScope,
            m_TypeName,
            clonedFunctions,
            clonedFunctionTemplates
        );
    }

    auto NormalImplNode::CreateBound() const -> Diagnosed<std::shared_ptr<const ImplBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

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
            std::move(diagnostics),
        };
    }

    auto NormalImplNode::DefineAssociations() const -> Expected<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        ACE_ASSERT(m_TypeName.Sections.back().TemplateArgs.empty());

        const auto optTypeSymbol = diagnostics.Collect(GetScope()->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName
        ));
        if (!optTypeSymbol.has_value())
        {
            return std::move(diagnostics);
        }

        optTypeSymbol.value()->GetSelfScope()->DefineAssociation(m_SelfScope);
        return Void{ std::move(diagnostics) };
    }
}
