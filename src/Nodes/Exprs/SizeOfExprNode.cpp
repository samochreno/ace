#include "Nodes/Exprs/SizeOfExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    SizeOfExprNode::SizeOfExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const TypeName& typeName
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope}, 
        m_TypeName{ typeName }
    {
    }

    auto SizeOfExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto SizeOfExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SizeOfExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        return {};
    }

    auto SizeOfExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const SizeOfExprNode>
    {
        return std::make_shared<const SizeOfExprNode>(
            m_SrcLocation,
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
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            typeSymbol
        );
    }

    auto SizeOfExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
