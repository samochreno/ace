#include "Nodes/Types/StructTypeNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Nodes/Vars/InstanceVarNode.hpp"
#include "BoundNodes/Types/StructTypeBoundNode.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    StructTypeNode::StructTypeNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& selfScope,
        const Ident& name,
        const std::vector<std::shared_ptr<const AttributeNode>>& attributes,
        const AccessModifier accessModifier,
        const std::vector<std::shared_ptr<const InstanceVarNode>>& vars
    ) : m_SrcLocation{ srcLocation },
        m_SelfScope{ selfScope },
        m_Name{ name },
        m_Attributes{ attributes },
        m_AccessModifier{ accessModifier },
        m_Vars{ vars }
    {
    }

    auto StructTypeNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StructTypeNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto StructTypeNode::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto StructTypeNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Attributes);
        AddChildren(children, m_Vars);

        return children;
    }

    auto StructTypeNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const StructTypeNode>
    {
        const auto selfScope = scope->GetOrCreateChild({});

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

        std::vector<std::shared_ptr<const InstanceVarNode>> clonedVars{};
        std::transform(
            begin(m_Vars),
            end  (m_Vars),
            back_inserter(clonedVars),
            [&](const std::shared_ptr<const InstanceVarNode>& var)
            {
                return var->CloneInScope(selfScope);
            }
        );

        return std::make_shared<const StructTypeNode>(
            m_SrcLocation,
            selfScope,
            m_Name,
            clonedAttributes,
            m_AccessModifier,
            clonedVars
        );
    }

    auto StructTypeNode::CloneInScopeType(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const ITypeNode>
    {
        return CloneInScope(scope);
    }

    auto StructTypeNode::CreateBound() const -> Diagnosed<std::shared_ptr<const StructTypeBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

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

        std::vector<std::shared_ptr<const InstanceVarBoundNode>> boundVars{};
        std::transform(begin(m_Vars), end(m_Vars), back_inserter(boundVars),
        [&](const std::shared_ptr<const InstanceVarNode>& var)
        {
            return diagnostics.Collect(var->CreateBound());
        });

        auto* const selfSymbol = GetScope()->ExclusiveResolveSymbol<StructTypeSymbol>(
            m_Name,
            m_SelfScope->CollectImplTemplateArgs(),
            m_SelfScope->CollectTemplateArgs()
        ).Unwrap();

        return Diagnosed
        {
            std::make_shared<const StructTypeBoundNode>(
                GetSrcLocation(),
                selfSymbol,
                boundAttributes,
                boundVars
            ),
            std::move(diagnostics),
        };
    }

    auto StructTypeNode::CreateBoundType() const -> Diagnosed<std::shared_ptr<const ITypeBoundNode>>
    {
        return CreateBound();
    }

    auto StructTypeNode::GetName() const -> const Ident&
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

    auto StructTypeNode::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<StructTypeSymbol>(
                m_SelfScope,
                m_Name,
                m_AccessModifier
            ),
            DiagnosticBag::Create(),
        };
    }
}
