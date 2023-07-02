#include "Nodes/Exprs/SizeOfExprNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    SizeOfExprNode::SizeOfExprNode(
        const std::shared_ptr<Scope>& t_scope,
        const TypeName& t_typeName
    ) : m_Scope{ t_scope}, 
        m_TypeName{ t_typeName }
    {
    }

    auto SizeOfExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SizeOfExprNode::GetChildren() const -> std::vector<const INode*>
    {
        return {};
    }

    auto SizeOfExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const SizeOfExprNode>
    {
        return std::make_shared<const SizeOfExprNode>(
            t_scope,
            m_TypeName
        );
    }

    auto SizeOfExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto SizeOfExprNode::CreateBound() const -> Expected<std::shared_ptr<const SizeOfExprBoundNode>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::make_shared<const SizeOfExprBoundNode>(
            m_Scope,
            typeSymbol
        );
    }

    auto SizeOfExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
