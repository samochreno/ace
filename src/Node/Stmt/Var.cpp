#include "Node/Stmt/Var.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Expr/Base.hpp"
#include "BoundNode/Stmt/Var.hpp"
#include "Diagnostics.hpp"
#include "Symbol/Var/Local.hpp"

namespace Ace::Node::Stmt
{
    auto Var::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        if (m_OptAssignedExpr.has_value())
        {
            AddChildren(children, m_OptAssignedExpr.value());
        }

        return children;
    }

    auto Var::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::Var>
    {
        const auto optAssignedExpr = [&]() -> std::optional<std::shared_ptr<const Node::Expr::IBase>>
        {
            if (!m_OptAssignedExpr.has_value())
                return std::nullopt;

            return m_OptAssignedExpr.value()->CloneInScopeExpr(t_scope);
        }();

        return std::make_shared<const Node::Stmt::Var>(
            t_scope,
            m_Name,
            m_TypeName,
            optAssignedExpr
        );
    }

    auto Var::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Var>>
    {
        auto* selfSymbol = m_Scope->ExclusiveResolveSymbol<Symbol::Var::Local>(m_Name).Unwrap();

        ACE_TRY(boundOptAssignedExpr, TransformExpectedOptional(m_OptAssignedExpr,
        [](const std::shared_ptr<const Node::Expr::IBase>& t_expr)
        {
            return t_expr->CreateBoundExpr();
        }));

        return std::make_shared<const BoundNode::Stmt::Var>(
            selfSymbol,
            boundOptAssignedExpr
            );
    }

    auto Var::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<Symbol::Type::IBase>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));
        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Var::Local>(
                m_Scope,
                m_Name,
                typeSymbol
            )
        };
    }
}
