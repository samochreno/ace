#include "Node/Expr/LogicalNegation.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expr//LogicalNegation.hpp"

namespace Ace::Node::Expr
{
    LogicalNegation::LogicalNegation(
        const std::shared_ptr<const Node::Expr::IBase>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto LogicalNegation::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto LogicalNegation::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto LogicalNegation::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::LogicalNegation>
    {
        return std::make_shared<const Node::Expr::LogicalNegation>(
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto LogicalNegation::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto LogicalNegation::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::LogicalNegation>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const BoundNode::Expr::LogicalNegation>(
            boundExpr
        );
    }

    auto LogicalNegation::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }
}
