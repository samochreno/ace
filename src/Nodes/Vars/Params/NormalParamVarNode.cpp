#include "Nodes/Vars/Params/NormalParamVarNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Vars/Params/NormalParamVarBoundNode.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    NormalParamVarNode::NormalParamVarNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        const Identifier& name,
        const TypeName& typeName,
        const std::vector<std::shared_ptr<const AttributeNode>>& attributes,
        const size_t index
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope },
        m_Name{ name },
        m_TypeName{ typeName },
        m_Attributes{ attributes },
        m_Index{ index }
    {
    }

    auto NormalParamVarNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto NormalParamVarNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalParamVarNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto NormalParamVarNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const NormalParamVarNode>
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

        return std::make_shared<const NormalParamVarNode>(
            m_SourceLocation,
            scope,
            m_Name,
            m_TypeName,
            clonedAttributes,
            m_Index
        );
    }

    auto NormalParamVarNode::CreateBound() const -> Expected<std::shared_ptr<const ParamVarBoundNode>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeNode>& attribute)
        {
            return attribute->CreateBound();
        }));

        auto* const selfSymbol = m_Scope->ExclusiveResolveSymbol<NormalParamVarSymbol>(
            m_Name
        ).Unwrap();

        return std::make_shared<const ParamVarBoundNode>(
            GetSourceLocation(),
            selfSymbol,
            boundAttributes
        );
    }

    auto NormalParamVarNode::GetName() const -> const Identifier&
    {
        return m_Name;
    }

    auto NormalParamVarNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalParamVarNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::ParamVar;
    }

    auto NormalParamVarNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto NormalParamVarNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));
        return std::unique_ptr<ISymbol>
        {
            std::make_unique<NormalParamVarSymbol>(
                m_Scope,
                m_Name,
                typeSymbol,
                m_Index
            )
        };
    }
}
