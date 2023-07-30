#include "Nodes/Vars/StaticVarNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Vars/StaticVarBoundNode.hpp"
#include "Symbols/Vars/StaticVarSymbol.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Vars/StaticVarSymbol.hpp"

namespace Ace
{
    StaticVarNode::StaticVarNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        const TypeName& typeName,
        const std::vector<std::shared_ptr<const AttributeNode>>& attributes,
        const AccessModifier accessModifier
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ name },
        m_TypeName{ typeName },
        m_Attributes{ attributes },
        m_AccessModifier{ accessModifier }
    {
    }

    auto StaticVarNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StaticVarNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StaticVarNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto StaticVarNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const StaticVarNode>
    {
        std::vector<std::shared_ptr<const AttributeNode>> clonedAttributes{};
        std::transform(
            begin(m_Attributes),
            end  (m_Attributes),
            back_inserter(clonedAttributes),
            [&](const std::shared_ptr<const AttributeNode>& attribute)
            {
                return attribute->CloneInScope(scope);
            }
        );

        return std::make_shared<const StaticVarNode>(
            m_SrcLocation,
            scope,
            m_Name,
            m_TypeName,
            clonedAttributes,
            m_AccessModifier
        );
    }

    auto StaticVarNode::CreateBound() const -> Expected<std::shared_ptr<const StaticVarBoundNode>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeNode>& attribute)
        {
            return attribute->CreateBound();
        }));

        auto* const selfSymbol = m_Scope->ExclusiveResolveSymbol<StaticVarSymbol>(
            m_Name
        ).Unwrap();

        return std::make_shared<const StaticVarBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            selfSymbol,
            boundAttributes
        );
    }

    auto StaticVarNode::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto StaticVarNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StaticVarNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::StaticVar;
    }

    auto StaticVarNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto StaticVarNode::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        DiagnosticBag diagnostics{};

        const auto expTypeSymbol = m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        );
        diagnostics.Add(expTypeSymbol);

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<StaticVarSymbol>(
                m_Scope,
                m_Name,
                m_AccessModifier,
                expTypeSymbol.UnwrapOr(GetCompilation()->ErrorTypeSymbol)
            ),
            diagnostics,
        };
    }
}
