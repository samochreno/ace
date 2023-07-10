#include "Nodes/Types/StructTypeNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Nodes/Vars/InstanceVarNode.hpp"
#include "BoundNodes/Types/StructTypeBoundNode.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    StructTypeNode::StructTypeNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_selfScope,
        const Identifier& t_name,
        const std::vector<std::shared_ptr<const AttributeNode>>& t_attributes,
        const AccessModifier t_accessModifier,
        const std::vector<std::shared_ptr<const InstanceVarNode>>& t_variables
    ) : m_SourceLocation{ t_sourceLocation },
        m_SelfScope{ t_selfScope },
        m_Name{ t_name },
        m_Attributes{ t_attributes },
        m_AccessModifier{ t_accessModifier },
        m_Vars{ t_variables }
    {
    }

    auto StructTypeNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto StructTypeNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto StructTypeNode::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto StructTypeNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Attributes);
        AddChildren(children, m_Vars);

        return children;
    }

    auto StructTypeNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const StructTypeNode>
    {
        const auto selfScope = t_scope->GetOrCreateChild({});

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

        std::vector<std::shared_ptr<const InstanceVarNode>> clonedVars{};
        std::transform(
            begin(m_Vars),
            end  (m_Vars),
            back_inserter(clonedVars),
            [&](const std::shared_ptr<const InstanceVarNode>& t_variable)
            {
                return t_variable->CloneInScope(selfScope);
            }
        );

        return std::make_shared<const StructTypeNode>(
            m_SourceLocation,
            selfScope,
            m_Name,
            clonedAttributes,
            m_AccessModifier,
            clonedVars
        );
    }

    auto StructTypeNode::CloneInScopeType(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const ITypeNode>
    {
        return CloneInScope(t_scope);
    }

    auto StructTypeNode::CreateBound() const -> Expected<std::shared_ptr<const StructTypeBoundNode>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeNode>& t_attribute)
        {
            return t_attribute->CreateBound();
        }));

        ACE_TRY(boundVars, TransformExpectedVector(m_Vars,
        [](const std::shared_ptr<const InstanceVarNode>& t_variable)
        {
            return t_variable->CreateBound();
        }));

        auto* const selfSymbol = GetScope()->ExclusiveResolveSymbol<StructTypeSymbol>(
            m_Name.String,
            m_SelfScope->CollectImplTemplateArgs(),
            m_SelfScope->CollectTemplateArgs()
        ).Unwrap();

        return std::make_shared<const StructTypeBoundNode>(
            selfSymbol,
            boundAttributes,
            boundVars
        );
    }

    auto StructTypeNode::CreateBoundType() const -> Expected<std::shared_ptr<const ITypeBoundNode>>
    {
        return CreateBound();
    }

    auto StructTypeNode::GetName() const -> const Identifier&
    {
        return m_Name;
    }

    auto StructTypeNode::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto StructTypeNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto StructTypeNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Struct;
    }

    auto StructTypeNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto StructTypeNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        return std::unique_ptr<ISymbol>
        {
            std::make_unique<StructTypeSymbol>(
                m_SelfScope,
                m_Name.String,
                m_AccessModifier
            )
        };
    }
}
