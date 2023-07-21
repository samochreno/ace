#include "Nodes/Exprs/SizeOfExprNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    SizeOfExprNode::SizeOfExprNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        const TypeName& typeName
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope}, 
        m_TypeName{ typeName }
    {
    }

    auto SizeOfExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const SizeOfExprNode>
    {
        return std::make_shared<const SizeOfExprNode>(
            m_SourceLocation,
            scope,
            m_TypeName
        );
    }

    auto SizeOfExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto SizeOfExprNode::CreateBound() const -> Expected<std::shared_ptr<const SizeOfExprBoundNode>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));
        return std::make_shared<const SizeOfExprBoundNode>(
            GetSourceLocation(),
            GetScope(),
            typeSymbol
        );
    }

    auto SizeOfExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
