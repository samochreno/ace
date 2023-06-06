#include "Node/Expression/BinaryUser.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expression/BinaryUser.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbol/Function.hpp"

namespace Ace::Node::Expression
{
    auto BinaryUser::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_LHSExpression);
        AddChildren(children, m_RHSExpression);

        return children;
    }

    auto BinaryUser::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::BinaryUser>
    {
        return std::make_shared<const Node::Expression::BinaryUser>(
            m_LHSExpression->CloneInScopeExpression(t_scope),
            m_RHSExpression->CloneInScopeExpression(t_scope),
            m_Operator
        );
    }

    auto BinaryUser::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::BinaryUser>>
    {
        ACE_TRY(boundLHSExpression, m_LHSExpression->CreateBoundExpression());
        ACE_TRY(boundRHSExpression, m_RHSExpression->CreateBoundExpression());

        auto* const lhsTypeSymbol = boundLHSExpression->GetTypeInfo().Symbol;
        auto* const rhsTypeSymbol = boundRHSExpression->GetTypeInfo().Symbol;

        const auto operatorNameIt = SpecialIdentifier::Operator::BinaryNameMap.find(m_Operator);
        ACE_TRY_ASSERT(operatorNameIt != end(SpecialIdentifier::Operator::BinaryNameMap));

        auto lhsName = lhsTypeSymbol->CreateFullyQualifiedName();
        lhsName.Sections.emplace_back(operatorNameIt->second);

        auto rhsName = rhsTypeSymbol->CreateFullyQualifiedName();
        rhsName.Sections.emplace_back(operatorNameIt->second);

        const std::vector argumentTypeSymbols{ lhsTypeSymbol, rhsTypeSymbol };

        const auto expLHSOperatorSymbol = GetScope()->ResolveStaticSymbol<Symbol::Function>(
            lhsName,
            Scope::CreateArgumentTypes(argumentTypeSymbols)
        );
        const auto expRHSOperatorSymbol = GetScope()->ResolveStaticSymbol<Symbol::Function>(
            rhsName,
            Scope::CreateArgumentTypes(argumentTypeSymbols)
        );

        ACE_TRY_ASSERT(expLHSOperatorSymbol || expRHSOperatorSymbol);

        if (expLHSOperatorSymbol && expRHSOperatorSymbol)
        {
            ACE_TRY_ASSERT(expLHSOperatorSymbol.Unwrap() == expRHSOperatorSymbol.Unwrap());
        }

        auto* const operatorSymbol = expLHSOperatorSymbol ?
            expLHSOperatorSymbol.Unwrap() :
            expRHSOperatorSymbol.Unwrap();

        return std::make_shared<const BoundNode::Expression::BinaryUser>(
            boundLHSExpression,
            boundRHSExpression,
            operatorSymbol
        );
    }
}
