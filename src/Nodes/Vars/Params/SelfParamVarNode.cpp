#include "Nodes/Vars/Params/SelfParamVarNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "SpecialIdentifier.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Vars/Params/SelfParamVarBoundNode.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Symbols/Symbol.hpp"
#include "Identifier.hpp"

namespace Ace
{
    SelfParamVarNode::SelfParamVarNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const SymbolName& t_typeName
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_Name{ t_sourceLocation, SpecialIdentifier::Self },
        m_TypeName{ t_typeName, std::vector{ TypeNameModifier::Reference } }
    {
    }

    auto SelfParamVarNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto SelfParamVarNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SelfParamVarNode::GetChildren() const -> std::vector<const INode*>
    {
        return {};
    }

    auto SelfParamVarNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const SelfParamVarNode>
    {
        return std::make_shared<const SelfParamVarNode>(
            m_SourceLocation,
            t_scope,
            m_TypeName.SymbolName
        );
    }

    auto SelfParamVarNode::CreateBound() const -> Expected<std::shared_ptr<const SelfParamVarBoundNode>>
    {
        auto* const selfSymbol = m_Scope->ExclusiveResolveSymbol<SelfParamVarSymbol>(
            m_Name.String
        ).Unwrap();

        return std::make_shared<const SelfParamVarBoundNode>(
            GetSourceLocation(),
            selfSymbol
        );
    }

    auto SelfParamVarNode::GetName() const -> const Identifier&
    {
        return m_Name;
    }

    auto SelfParamVarNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SelfParamVarNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Function;
    }

    auto SelfParamVarNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto SelfParamVarNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::unique_ptr<ISymbol>
        {
            std::make_unique<SelfParamVarSymbol>(
                m_SourceLocation,
                m_Scope,
                typeSymbol
            )
        };
    }
}
