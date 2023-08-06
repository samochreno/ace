#include "Nodes/Exprs/AddressOfExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/AddressOfExprBoundNode.hpp"

namespace Ace
{
    AddressOfExprNode::AddressOfExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& expr
    ) : m_Expr{ expr }
    {
    }

    auto AddressOfExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_Expr->GetSrcLocation();
    }

    auto AddressOfExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto AddressOfExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto AddressOfExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const AddressOfExprNode>
    {
        return std::make_shared<const AddressOfExprNode>(
            m_SrcLocation,
            m_Expr->CloneInScopeExpr(scope)
        );
    }

    auto AddressOfExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto AddressOfExprNode::CreateBound() const -> Diagnosed<std::shared_ptr<const AddressOfExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto dgnBoundExpr = m_Expr->CreateBoundExpr();
        diagnostics.Add(dgnBoundExpr);

        return Diagnosed
        {
            std::make_shared<const AddressOfExprBoundNode>(
                GetSrcLocation(),
                dgnBoundExpr.Unwrap()
            ),
            diagnostics,
        };
    }

    auto AddressOfExprNode::CreateBoundExpr() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
