#include "Nodes/Vars/InstanceVarNode.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Var/Normal/Instance.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"

namespace Ace
{
    InstanceVarNode::InstanceVarNode(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        const TypeName& t_typeName,
        const std::vector<std::shared_ptr<const AttributeNode>>& t_attributes,
        const AccessModifier& t_accessModifier,
        const size_t& t_index
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_TypeName{ t_typeName },
        m_Attributes{ t_attributes },
        m_AccessModifier{ t_accessModifier },
        m_Index{ t_index }
    {
    }

    auto InstanceVarNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto InstanceVarNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto InstanceVarNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const InstanceVarNode>
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

        return std::make_shared<const InstanceVarNode>(
            t_scope,
            m_Name,
            m_TypeName,
            clonedAttributes,
            m_AccessModifier,
            m_Index
        );
    }

    auto InstanceVarNode::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Var::Normal::Instance>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeNode>& t_attribute)
        {
            return t_attribute->CreateBound();
        }));

        auto* const selfSymbol =
            m_Scope->ExclusiveResolveSymbol<InstanceVarSymbol>(m_Name).Unwrap();

        return std::make_shared<const BoundNode::Var::Normal::Instance>(
            selfSymbol,
            boundAttributes
        );
    }

    auto InstanceVarNode::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto InstanceVarNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto InstanceVarNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::InstanceVar;
    }

    auto InstanceVarNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto InstanceVarNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::unique_ptr<ISymbol>
        {
            std::make_unique<InstanceVarSymbol>(
                m_Scope,
                m_Name,
                m_AccessModifier,
                typeSymbol,
                m_Index
            )
        };
    }
}
