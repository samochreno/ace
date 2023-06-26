#include "Node/Expr/SizeOf.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace::Node::Expr
{
    SizeOf::SizeOf(
        const std::shared_ptr<Scope>& t_scope,
        const TypeName& t_typeName
    ) : m_Scope{ t_scope}, 
        m_TypeName{ t_typeName }
    {
    }

    auto SizeOf::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SizeOf::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto SizeOf::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::SizeOf>
    {
        return std::make_shared<const Node::Expr::SizeOf>(
            t_scope,
            m_TypeName
        );
    }

    auto SizeOf::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto SizeOf::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::SizeOf>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::make_shared<const BoundNode::Expr::SizeOf>(
            m_Scope,
            typeSymbol
        );
    }

    auto SizeOf::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }
}
