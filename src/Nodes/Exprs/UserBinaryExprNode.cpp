#include "Nodes/Exprs/UserBinaryExprNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/UserBinaryExprBoundNode.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    UserBinaryExprNode::UserBinaryExprNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<const IExprNode>& t_lhsExpr,
        const std::shared_ptr<const IExprNode>& t_rhsExpr,
        const Op& t_op
    ) : m_SourceLocation{ t_sourceLocation },
        m_LHSExpr{ t_lhsExpr },
        m_RHSExpr{ t_rhsExpr },
        m_Op{ t_op }
    {
    }

    auto UserBinaryExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto UserBinaryExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto UserBinaryExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto UserBinaryExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const UserBinaryExprNode>
    {
        return std::make_shared<const UserBinaryExprNode>(
            m_SourceLocation,
            m_LHSExpr->CloneInScopeExpr(t_scope),
            m_RHSExpr->CloneInScopeExpr(t_scope),
            m_Op
        );
    }

    auto UserBinaryExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto UserBinaryExprNode::CreateBound() const -> Expected<std::shared_ptr<const UserBinaryExprBoundNode>>
    {
        ACE_TRY(boundLHSExpr, m_LHSExpr->CreateBoundExpr());
        ACE_TRY(boundRHSExpr, m_RHSExpr->CreateBoundExpr());

        auto* const lhsTypeSymbol = boundLHSExpr->GetTypeInfo().Symbol;
        auto* const rhsTypeSymbol = boundRHSExpr->GetTypeInfo().Symbol;

        const auto& opNameMap =
            SpecialIdentifier::Op::BinaryNameMap;

        const auto opNameIt = opNameMap.find(m_Op.TokenKind);
        ACE_TRY_ASSERT(opNameIt != end(opNameMap));

        auto lhsName = lhsTypeSymbol->CreateFullyQualifiedName(
            m_Op.SourceLocation
        );
        lhsName.Sections.emplace_back(Identifier{
            m_Op.SourceLocation,
            opNameIt->second,
        });

        auto rhsName = rhsTypeSymbol->CreateFullyQualifiedName(
            m_Op.SourceLocation
        );
        rhsName.Sections.emplace_back(Identifier{
            m_Op.SourceLocation,
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
            [](const TypeInfo& t_typeInfo) { return t_typeInfo.Symbol; }
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
            GetSourceLocation(),
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
