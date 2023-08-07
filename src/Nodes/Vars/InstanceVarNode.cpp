#include "Nodes/Vars/InstanceVarNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Vars/InstanceVarBoundNode.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"

namespace Ace
{
    InstanceVarNode::InstanceVarNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        const TypeName& typeName,
        const std::vector<std::shared_ptr<const AttributeNode>>& attributes,
        const AccessModifier accessModifier,
        const size_t index
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ name },
        m_TypeName{ typeName },
        m_Attributes{ attributes },
        m_AccessModifier{ accessModifier },
        m_Index{ index }
    {
    }

    auto InstanceVarNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto InstanceVarNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto InstanceVarNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto InstanceVarNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const InstanceVarNode>
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

        return std::make_shared<const InstanceVarNode>(
            m_SrcLocation,
            scope,
            m_Name,
            m_TypeName,
            clonedAttributes,
            m_AccessModifier,
            m_Index
        );
    }

    auto InstanceVarNode::CreateBound() const -> Diagnosed<std::shared_ptr<const InstanceVarBoundNode>>
    {
        DiagnosticBag diagnostics{};

        std::vector<std::shared_ptr<const AttributeBoundNode>> boundAttributes{};
        std::transform(
            begin(m_Attributes),
            end  (m_Attributes),
            back_inserter(boundAttributes),
            [&](const std::shared_ptr<const AttributeNode>& attribute)
            {
                return diagnostics.Collect(attribute->CreateBound());
            }
        );

        auto* const selfSymbol = m_Scope->ExclusiveResolveSymbol<InstanceVarSymbol>(
            m_Name
        ).Unwrap();

        return Diagnosed
        {
            std::make_shared<const InstanceVarBoundNode>(
                GetSrcLocation(),
                selfSymbol,
                boundAttributes
            ),
            diagnostics,
        };
    }

    auto InstanceVarNode::GetName() const -> const Ident&
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

    auto InstanceVarNode::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        DiagnosticBag diagnostics{};

        const auto optTypeSymbol = diagnostics.Collect(m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<InstanceVarSymbol>(
                m_Scope,
                m_Name,
                m_AccessModifier,
                typeSymbol,
                m_Index
            ),
            diagnostics,
        };
    }
}
