#include "Nodes/Vars/Params/SelfParamVarNode.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Nodes/AttributeNode.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Var/Param/Self.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    SelfParamVarNode::SelfParamVarNode(
        const std::shared_ptr<Scope>& t_scope,
        const SymbolName& t_typeName
    ) : m_Scope{ t_scope },
        m_Name{ SpecialIdentifier::Self },
        m_TypeName{ t_typeName, std::vector{ TypeNameModifier::Reference } }
    {
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
            t_scope,
            m_TypeName.SymbolName
        );
    }

    auto SelfParamVarNode::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Var::Param::Self>>
    {
        auto* const selfSymbol =
            m_Scope->ExclusiveResolveSymbol<SelfParamVarSymbol>(m_Name).Unwrap();

        return std::make_shared<const BoundNode::Var::Param::Self>(
            selfSymbol
        );
    }

    auto SelfParamVarNode::GetName() const -> const std::string&
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
                m_Scope,
                typeSymbol
            )
        };
    }
}
