#include "Nodes/Vars/StaticVarNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Diagnostics.hpp"
#include "BoundNodes/Vars/StaticVarBoundNode.hpp"
#include "Symbols/Vars/StaticVarSymbol.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Vars/StaticVarSymbol.hpp"

namespace Ace
{
    StaticVarNode::StaticVarNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const Identifier& t_name,
        const TypeName& t_typeName,
        const std::vector<std::shared_ptr<const AttributeNode>>& t_attributes,
        const AccessModifier t_accessModifier
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_Name{ t_name },
        m_TypeName{ t_typeName },
        m_Attributes{ t_attributes },
        m_AccessModifier{ t_accessModifier }
    {
    }

    auto StaticVarNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto StaticVarNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StaticVarNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto StaticVarNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const StaticVarNode>
    {
        std::vector<std::shared_ptr<const AttributeNode>> clonedAttributes{};
        std::transform(
            begin(m_Attributes),
            end  (m_Attributes),
            back_inserter(clonedAttributes),
            [&](const std::shared_ptr<const AttributeNode>& t_attribute)
            {
                return t_attribute->CloneInScope(t_scope);
            }
        );

        return std::make_shared<const StaticVarNode>(
            m_SourceLocation,
            t_scope,
            m_Name,
            m_TypeName,
            clonedAttributes,
            m_AccessModifier
        );
    }

    auto StaticVarNode::CreateBound() const -> Expected<std::shared_ptr<const StaticVarBoundNode>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeNode>& t_attribute)
        {
            return t_attribute->CreateBound();
        }));

        auto* const selfSymbol = m_Scope->ExclusiveResolveSymbol<StaticVarSymbol>(
            m_Name.String
        ).Unwrap();

        return std::make_shared<const StaticVarBoundNode>(
            selfSymbol,
            boundAttributes
        );
    }

    auto StaticVarNode::GetName() const -> const Identifier&
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

    auto StaticVarNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::unique_ptr<ISymbol>
        {
            std::make_unique<StaticVarSymbol>(
                m_Scope,
                m_Name,
                m_AccessModifier,
                typeSymbol
            )
        };
    }
}
