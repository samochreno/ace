#include "Nodes/Vars/Params/NormalParamVarNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Vars/Params/NormalParamVarBoundNode.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    NormalParamVarNode::NormalParamVarNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        const TypeName& typeName,
        const std::vector<std::shared_ptr<const AttributeNode>>& attributes,
        const size_t index
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ name },
        m_TypeName{ typeName },
        m_Attributes{ attributes },
        m_Index{ index }
    {
    }

    auto NormalParamVarNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto NormalParamVarNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalParamVarNode::CollectChildren() const -> std::vector<const INode*>
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
            m_SrcLocation,
            scope,
            m_Name,
            m_TypeName,
            clonedAttributes,
            m_Index
        );
    }

    auto NormalParamVarNode::CreateBound() const -> Expected<std::shared_ptr<const NormalParamVarBoundNode>>
    {
        ACE_TRY(boundAttributes, TransformExpectedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeNode>& attribute)
        {
            return attribute->CreateBound();
        }));

        auto* const selfSymbol = m_Scope->ExclusiveResolveSymbol<NormalParamVarSymbol>(
            m_Name
        ).Unwrap();

        return std::make_shared<const NormalParamVarBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            selfSymbol,
            boundAttributes
        );
    }

    auto NormalParamVarNode::GetName() const -> const Ident&
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

    auto NormalParamVarNode::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        DiagnosticBag diagnostics{};

        const auto expTypeSymbol = m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        );
        diagnostics.Add(expTypeSymbol);

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<NormalParamVarSymbol>(
                m_Scope,
                m_Name,
                expTypeSymbol.UnwrapOr(GetCompilation()->ErrorTypeSymbol),
                m_Index
            ),
            diagnostics,
        };
    }
}
