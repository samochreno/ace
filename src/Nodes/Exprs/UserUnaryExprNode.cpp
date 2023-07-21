#include "Nodes/Exprs/UserUnaryExprNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "SpecialIdentifier.hpp"
#include "BoundNodes/Exprs/UserUnaryExprBoundNode.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    UserUnaryExprNode::UserUnaryExprNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprNode>& expr,
        const Op op
    ) : m_SourceLocation{ sourceLocation },
        m_Expr{ expr },
        m_Op{ op }
    {
    }

    auto UserUnaryExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const UserUnaryExprNode>
    {
        return std::make_unique<UserUnaryExprNode>(
            m_SourceLocation,
            m_Expr->CloneInScopeExpr(scope),
            m_Op
        );
    }

    auto UserUnaryExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto UserUnaryExprNode::CreateBound() const -> Expected<std::shared_ptr<const UserUnaryExprBoundNode>>
    {
        ACE_TRY(boundExpresssion, m_Expr->CreateBoundExpr());

        const auto& opNameMap =
            SpecialIdentifier::Op::UnaryNameMap;

        const auto opNameIt = opNameMap.find(m_Op.TokenKind);
        ACE_TRY_ASSERT(opNameIt != end(opNameMap));

        auto* const typeSymbol = boundExpresssion->GetTypeInfo().Symbol;

        auto opFullName = typeSymbol->CreateFullyQualifiedName(
            m_Op.SourceLocation
        );
        opFullName.Sections.emplace_back(Identifier{
            m_Op.SourceLocation,
            opNameIt->second,
        });

        ACE_TRY(opSymbol, GetScope()->ResolveStaticSymbol<FunctionSymbol>(
            opFullName,
            Scope::CreateArgTypes(typeSymbol)
        ));

        return std::make_shared<const UserUnaryExprBoundNode>(
            GetSourceLocation(),
            boundExpresssion,
            opSymbol
        );
    }

    auto UserUnaryExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
