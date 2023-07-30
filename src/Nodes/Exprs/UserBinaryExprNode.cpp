#include "Nodes/Exprs/UserBinaryExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/UserBinaryExprBoundNode.hpp"
#include "SpecialIdent.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    UserBinaryExprNode::UserBinaryExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& lhsExpr,
        const std::shared_ptr<const IExprNode>& rhsExpr,
        const Op& op
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr },
        m_Op{ op }
    {
    }

    auto UserBinaryExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UserBinaryExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto UserBinaryExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto UserBinaryExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const UserBinaryExprNode>
    {
        return std::make_shared<const UserBinaryExprNode>(
            m_SrcLocation,
            m_LHSExpr->CloneInScopeExpr(scope),
            m_RHSExpr->CloneInScopeExpr(scope),
            m_Op
        );
    }

    auto UserBinaryExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto UserBinaryExprNode::CreateBound() const -> Expected<std::shared_ptr<const UserBinaryExprBoundNode>>
    {
        ACE_TRY(boundLHSExpr, m_LHSExpr->CreateBoundExpr());
        ACE_TRY(boundRHSExpr, m_RHSExpr->CreateBoundExpr());

        auto* const lhsTypeSymbol = boundLHSExpr->GetTypeInfo().Symbol;
        auto* const rhsTypeSymbol = boundRHSExpr->GetTypeInfo().Symbol;

        const auto& opNameMap =
            SpecialIdent::Op::BinaryNameMap;

        const auto opNameIt = opNameMap.find(m_Op.TokenKind);
        ACE_TRY_ASSERT(opNameIt != end(opNameMap));

        auto lhsName = lhsTypeSymbol->CreateFullyQualifiedName(
            m_Op.SrcLocation
        );
        lhsName.Sections.emplace_back(Ident{
            m_Op.SrcLocation,
            opNameIt->second,
        });

        auto rhsName = rhsTypeSymbol->CreateFullyQualifiedName(
            m_Op.SrcLocation
        );
        rhsName.Sections.emplace_back(Ident{
            m_Op.SrcLocation,
            opNameIt->second,
        });

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
            [](const TypeInfo& typeInfo) { return typeInfo.Symbol; }
        );

        const auto expLHSOpSymbol = [&]() -> Expected<FunctionSymbol*>
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
        const auto expRHSOpSymbol = [&]() -> Expected<FunctionSymbol*>
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

        ACE_TRY_ASSERT(expLHSOpSymbol || expRHSOpSymbol);

        if (expLHSOpSymbol && expRHSOpSymbol)
        {
            ACE_TRY_ASSERT(
                expLHSOpSymbol.Unwrap() ==
                expRHSOpSymbol.Unwrap()
            );
        }

        auto* const opSymbol = expLHSOpSymbol ?
            expLHSOpSymbol.Unwrap() :
            expRHSOpSymbol.Unwrap();

        return std::make_shared<const UserBinaryExprBoundNode>(
            GetSrcLocation(),
            boundLHSExpr,
            boundRHSExpr,
            opSymbol
        );
    }

    auto UserBinaryExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
