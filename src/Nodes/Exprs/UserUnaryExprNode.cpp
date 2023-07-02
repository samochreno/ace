#include "Nodes/Exprs/UserUnaryExprNode.hpp"

#include <memory>
#include <vector>

#include "SpecialIdentifier.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNodes/Exprs/UserUnaryExprBoundNode.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    UserUnaryExprNode::UserUnaryExprNode(
        const std::shared_ptr<const IExprNode>& t_expr,
        const TokenKind t_operator
    ) : m_Expr{ t_expr },
        m_Operator{ t_operator }
    {
    }

    auto UserUnaryExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UserUnaryExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto UserUnaryExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const UserUnaryExprNode>
    {
        return std::make_unique<UserUnaryExprNode>(
            m_Expr->CloneInScopeExpr(t_scope),
            m_Operator
        );
    }

    auto UserUnaryExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto UserUnaryExprNode::CreateBound() const -> Expected<std::shared_ptr<const UserUnaryExprBoundNode>>
    {
        ACE_TRY(boundExpresssion, m_Expr->CreateBoundExpr());

        const auto operatorNameIt =
            SpecialIdentifier::Operator::UnaryNameMap.find(m_Operator);
        ACE_TRY_ASSERT(
            operatorNameIt != end(SpecialIdentifier::Operator::UnaryNameMap)
        );

        auto* const typeSymbol = boundExpresssion->GetTypeInfo().Symbol;

        auto operatorFullName = typeSymbol->CreateFullyQualifiedName();
        operatorFullName.Sections.emplace_back(operatorNameIt->second);

        ACE_TRY(operatorSymbol, GetScope()->ResolveStaticSymbol<FunctionSymbol>(
            operatorFullName,
            Scope::CreateArgTypes(typeSymbol)
        ));

        return std::make_shared<const UserUnaryExprBoundNode>(
            boundExpresssion,
            operatorSymbol
        );
    }

    auto UserUnaryExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
