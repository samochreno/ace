#include "Node/Expression/UserBinary.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expression/UserBinary.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbol/Function.hpp"

namespace Ace::Node::Expression
{
    auto UserBinary::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_LHSExpression);
        AddChildren(children, m_RHSExpression);

        return children;
    }

    auto UserBinary::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::UserBinary>
    {
        return std::make_shared<const Node::Expression::UserBinary>(
            m_LHSExpression->CloneInScopeExpression(t_scope),
            m_RHSExpression->CloneInScopeExpression(t_scope),
            m_Operator
        );
    }

    auto UserBinary::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::UserBinary>>
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

        const std::vector<TypeInfo> argumentTypeInfos
        {
            boundLHSExpression->GetTypeInfo(),
            boundRHSExpression->GetTypeInfo(),
        };

        std::vector<Symbol::Type::IBase*> argumentTypeSymbols{};
        std::transform(
            begin(argumentTypeInfos),
            end  (argumentTypeInfos),
            back_inserter(argumentTypeSymbols),
            [](const TypeInfo& t_typeInfo) { return t_typeInfo.Symbol; }
        );

        const auto expLHSOperatorSymbol = [&]() -> Expected<Symbol::Function*>
        {
            ACE_TRY(symbol, GetScope()->ResolveStaticSymbol<Symbol::Function>(
                lhsName,
                Scope::CreateArgumentTypes(argumentTypeSymbols)
            ));

            ACE_TRY_ASSERT(AreTypesConvertible(
                GetScope(),
                argumentTypeInfos,
                symbol->CollectArgumentTypeInfos()
            ));

            return symbol;
        }();
        const auto expRHSOperatorSymbol = [&]() -> Expected<Symbol::Function*>
        {
            ACE_TRY(symbol, GetScope()->ResolveStaticSymbol<Symbol::Function>(
                rhsName,
                Scope::CreateArgumentTypes(argumentTypeSymbols)
            ));

            ACE_TRY_ASSERT(AreTypesConvertible(
                GetScope(),
                argumentTypeInfos,
                symbol->CollectArgumentTypeInfos()
            ));

            return symbol;
        }();

        ACE_TRY_ASSERT(expLHSOperatorSymbol || expRHSOperatorSymbol);

        if (expLHSOperatorSymbol && expRHSOperatorSymbol)
        {
            ACE_TRY_ASSERT(
                expLHSOperatorSymbol.Unwrap() ==
                expRHSOperatorSymbol.Unwrap()
            );
        }

        auto* const operatorSymbol = expLHSOperatorSymbol ?
            expLHSOperatorSymbol.Unwrap() :
            expRHSOperatorSymbol.Unwrap();

        return std::make_shared<const BoundNode::Expression::UserBinary>(
            boundLHSExpression,
            boundRHSExpression,
            operatorSymbol
        );
    }
}
