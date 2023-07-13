#include "Nodes/Exprs/DerefAsExprNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "BoundNodes/Exprs/DerefAsExprBoundNode.hpp"

namespace Ace
{
    DerefAsExprNode::DerefAsExprNode(
        const SourceLocation& t_sourceLocation,
        const TypeName& t_typeName, 
        const std::shared_ptr<const IExprNode>& t_expr
    ) : m_SourceLocation{ t_sourceLocation },
        m_TypeName{ t_typeName },
        m_Expr{ t_expr }
    {
    }

    auto DerefAsExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto DerefAsExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DerefAsExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DerefAsExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const DerefAsExprNode>
    {
        return std::make_shared<const DerefAsExprNode>(
            m_SourceLocation,
            m_TypeName,
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto DerefAsExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto DerefAsExprNode::CreateBound() const -> Expected<std::shared_ptr<const DerefAsExprBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        ACE_TRY(typeSymbol, GetScope()->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::make_shared<const DerefAsExprBoundNode>(
            boundExpr,
            typeSymbol
        );
    }

    auto DerefAsExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
