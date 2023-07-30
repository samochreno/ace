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

    auto AddressOfExprNode::CreateBound() const -> Expected<std::shared_ptr<const AddressOfExprBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const AddressOfExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            boundExpr
        );
    }

    auto AddressOfExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
