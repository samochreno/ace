#include "Nodes/Exprs/CastExprNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    CastExprNode::CastExprNode(
        const SourceLocation& t_sourceLocation,
        const TypeName& t_typeName,
        const std::shared_ptr<const IExprNode>& t_expr
    ) : m_SourceLocation{ t_sourceLocation },
        m_TypeName{ t_typeName },
        m_Expr{ t_expr }
    {
    }

    auto CastExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto CastExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto CastExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto CastExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const CastExprNode>
    {
        return std::make_shared<const CastExprNode>(
            m_SourceLocation,
            m_TypeName,
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto CastExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto CastExprNode::CreateBound() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());

        ACE_TRY(typeSymbol, GetScope()->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        ACE_TRY(mchConvertedBoundExpr, CreateExplicitlyConverted(
            boundExpr, 
            TypeInfo{ typeSymbol, ValueKind::R }
        ));

        return mchConvertedBoundExpr.Value;
    }

    auto CastExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
