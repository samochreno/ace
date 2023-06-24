#include "Node/Expr/UserUnary.hpp"

#include <memory>
#include <vector>

#include "SpecialIdentifier.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expr/UserUnary.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbol/Function.hpp"

namespace Ace::Node::Expr
{
    auto UserUnary::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto UserUnary::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::UserUnary>
    {
        return std::make_unique<Node::Expr::UserUnary>(
            m_Expr->CloneInScopeExpr(t_scope),
            m_Operator
        );
    }

    auto UserUnary::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::UserUnary>>
    {
        ACE_TRY(boundExpresssion, m_Expr->CreateBoundExpr());

        const auto operatorNameIt = SpecialIdentifier::Operator::UnaryNameMap.find(m_Operator);
        ACE_TRY_ASSERT(operatorNameIt != end(SpecialIdentifier::Operator::UnaryNameMap));

        auto* const typeSymbol = boundExpresssion->GetTypeInfo().Symbol;

        auto operatorFullName = typeSymbol->CreateFullyQualifiedName();
        operatorFullName.Sections.emplace_back(operatorNameIt->second);

        ACE_TRY(operatorSymbol, GetScope()->ResolveStaticSymbol<Symbol::Function>(
            operatorFullName,
            Scope::CreateArgTypes(typeSymbol)
        ));

        return std::make_shared<const BoundNode::Expr::UserUnary>(
            boundExpresssion,
            operatorSymbol
        );
    }
}
