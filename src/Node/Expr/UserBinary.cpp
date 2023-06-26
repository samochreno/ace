#include "Node/Expr/UserBinary.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expr/UserBinary.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace::Node::Expr
{
    auto UserBinary::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto UserBinary::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::UserBinary>
    {
        return std::make_shared<const Node::Expr::UserBinary>(
            m_LHSExpr->CloneInScopeExpr(t_scope),
            m_RHSExpr->CloneInScopeExpr(t_scope),
            m_Operator
        );
    }

    auto UserBinary::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::UserBinary>>
    {
        ACE_TRY(boundLHSExpr, m_LHSExpr->CreateBoundExpr());
        ACE_TRY(boundRHSExpr, m_RHSExpr->CreateBoundExpr());

        auto* const lhsTypeSymbol = boundLHSExpr->GetTypeInfo().Symbol;
        auto* const rhsTypeSymbol = boundRHSExpr->GetTypeInfo().Symbol;

        const auto operatorNameIt = SpecialIdentifier::Operator::BinaryNameMap.find(m_Operator);
        ACE_TRY_ASSERT(operatorNameIt != end(SpecialIdentifier::Operator::BinaryNameMap));

        auto lhsName = lhsTypeSymbol->CreateFullyQualifiedName();
        lhsName.Sections.emplace_back(operatorNameIt->second);

        auto rhsName = rhsTypeSymbol->CreateFullyQualifiedName();
        rhsName.Sections.emplace_back(operatorNameIt->second);

        const std::vector<TypeInfo> argTypeInfos
        {
            boundLHSExpr->GetTypeInfo(),
            boundRHSExpr->GetTypeInfo(),
        };

        std::vector<ITypeSymbol*> argTypeSymbols{};
        std::transform(
            begin(argTypeInfos),
            end  (argTypeInfos),
            back_inserter(argTypeSymbols),
            [](const TypeInfo& t_typeInfo) { return t_typeInfo.Symbol; }
        );

        const auto expLHSOperatorSymbol = [&]() -> Expected<FunctionSymbol*>
        {
            ACE_TRY(symbol, GetScope()->ResolveStaticSymbol<FunctionSymbol>(
                lhsName,
                Scope::CreateArgTypes(argTypeSymbols)
            ));

            ACE_TRY_ASSERT(AreTypesConvertible(
                GetScope(),
                argTypeInfos,
                symbol->CollectArgTypeInfos()
            ));

            return symbol;
        }();
        const auto expRHSOperatorSymbol = [&]() -> Expected<FunctionSymbol*>
        {
            ACE_TRY(symbol, GetScope()->ResolveStaticSymbol<FunctionSymbol>(
                rhsName,
                Scope::CreateArgTypes(argTypeSymbols)
            ));

            ACE_TRY_ASSERT(AreTypesConvertible(
                GetScope(),
                argTypeInfos,
                symbol->CollectArgTypeInfos()
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

        return std::make_shared<const BoundNode::Expr::UserBinary>(
            boundLHSExpr,
            boundRHSExpr,
            operatorSymbol
        );
    }
}
