#include "Node/Stmt/Return.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "Scope.hpp"
#include "Node/Expr/Base.hpp"
#include "BoundNode/Stmt/Return.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Stmt
{
    auto Return::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        if (m_OptExpr.has_value())
        {
            AddChildren(children, m_OptExpr.value());
        }

        return children;
    }

    auto Return::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::Return>
    {
        const auto clonedOptExpr = [&]() -> std::optional<std::shared_ptr<const Node::Expr::IBase>>
        {
            if (!m_OptExpr.has_value())
            {
                return std::nullopt;
            }

            return m_OptExpr.value()->CloneInScopeExpr(t_scope);
        }();

        return std::make_shared<const Node::Stmt::Return>(
            t_scope,
            clonedOptExpr
        );
    }

    auto Return::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Return>>
    {
        ACE_TRY(boundOptExpr, TransformExpectedOptional(m_OptExpr,
        [](const std::shared_ptr<const Node::Expr::IBase>& t_expr)
        {
            return t_expr->CreateBoundExpr();
        }));

        return std::make_shared<const BoundNode::Stmt::Return>(
            m_Scope,
            boundOptExpr
            );
    }
}
