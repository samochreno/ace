#include "Node/Var/Param/Self.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Attribute.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Var/Param/Self.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace::Node::Var::Param
{
    Self::Self(
        const std::shared_ptr<Scope>& t_scope,
        const SymbolName& t_typeName
    ) : m_Scope{ t_scope },
        m_Name{ SpecialIdentifier::Self },
        m_TypeName{ t_typeName, std::vector{ TypeNameModifier::Reference } }
    {
    }

    auto Self::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }


    auto Self::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto Self::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Var::Param::Self>
    {
        return std::make_shared<const Node::Var::Param::Self>(
            t_scope,
            m_TypeName.SymbolName
        );
    }

    auto Self::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Var::Param::Self>>
    {
        auto* const selfSymbol =
            m_Scope->ExclusiveResolveSymbol<SelfParamVarSymbol>(m_Name).Unwrap();

        return std::make_shared<const BoundNode::Var::Param::Self>(
            selfSymbol
        );
    }

    auto Self::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Self::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Self::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Function;
    }

    auto Self::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto Self::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
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
