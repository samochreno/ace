#include "Nodes/Exprs/BoxExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "BoundNodes/Exprs/BoxExprBoundNode.hpp"

namespace Ace
{
    BoxExprNode::BoxExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto BoxExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto BoxExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto BoxExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto BoxExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const BoxExprNode>
    {
        return std::make_shared<const BoxExprNode>(
            m_SrcLocation,
            m_Expr->CloneInScopeExpr(scope)
        );
    }

    auto BoxExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto BoxExprNode::CreateBound() const -> Diagnosed<std::shared_ptr<const BoxExprBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto boundExpr = diagnostics.Collect(m_Expr->CreateBoundExpr());

        return Diagnosed
        {
            std::make_shared<const BoxExprBoundNode>(
                GetSrcLocation(),
                boundExpr
            ),
            std::move(diagnostics),
        };
    }

    auto BoxExprNode::CreateBoundExpr() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
