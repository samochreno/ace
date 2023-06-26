#include "Node/Expr/UserUnary.hpp"

#include <memory>
#include <vector>

#include "SpecialIdentifier.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expr/UserUnary.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace::Node::Expr
{
    UserUnary::UserUnary(
        const std::shared_ptr<const Node::Expr::IBase>& t_expr,
        const TokenKind& t_operator
    ) : m_Expr{ t_expr },
        m_Operator{ t_operator }
    {
    }

    auto UserUnary::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UserUnary::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto UserUnary::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::UserUnary>
    {
        return std::make_unique<Node::Expr::UserUnary>(
            m_Expr->CloneInScopeExpr(t_scope),
            m_Operator
        );
    }

    auto UserUnary::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto UserUnary::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::UserUnary>>
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

        return std::make_shared<const BoundNode::Expr::UserUnary>(
            boundExpresssion,
            operatorSymbol
        );
    }

    auto UserUnary::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }
}
