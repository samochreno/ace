#include "Nodes/Exprs/UnboxExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "BoundNodes/Exprs/BoxExprBoundNode.hpp"

namespace Ace
{
    UnboxExprNode::UnboxExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto UnboxExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UnboxExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UnboxExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto UnboxExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const UnboxExprNode>
    {
        return std::make_shared<const UnboxExprNode>(
            m_SrcLocation,
            m_Expr->CloneInScopeExpr(scope)
        );
    }

    auto UnboxExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto UnboxExprNode::CreateBound() const -> std::shared_ptr<const UnboxExprBoundNode>
    {
        const auto boundExpr = m_Expr->CreateBoundExpr();

        return std::make_shared<const UnboxExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            boundExpr
        );
    }

    auto UnboxExprNode::CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateBound();
    }
}
