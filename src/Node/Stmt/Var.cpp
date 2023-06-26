#include "Node/Stmt/Var.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Expr/Base.hpp"
#include "BoundNode/Stmt/Var.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"

namespace Ace::Node::Stmt
{
    Var::Var(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        const TypeName& t_typeName,
        const std::optional<std::shared_ptr<const Node::Expr::IBase>>& t_optAssignedExpr
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_TypeName{ t_typeName },
        m_OptAssignedExpr{ t_optAssignedExpr }
    {
    }

    auto Var::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Var::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        if (m_OptAssignedExpr.has_value())
        {
            AddChildren(children, m_OptAssignedExpr.value());
        }

        return children;
    }

    auto Var::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::Var>
    {
        const auto optAssignedExpr = [&]() -> std::optional<std::shared_ptr<const Node::Expr::IBase>>
        {
            if (!m_OptAssignedExpr.has_value())
            {
                return std::nullopt;
            }

            return m_OptAssignedExpr.value()->CloneInScopeExpr(t_scope);
        }();

        return std::make_shared<const Node::Stmt::Var>(
            t_scope,
            m_Name,
            m_TypeName,
            optAssignedExpr
        );
    }

    auto Var::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto Var::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Var>>
    {
        auto* selfSymbol =
            m_Scope->ExclusiveResolveSymbol<LocalVarSymbol>(m_Name).Unwrap();

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

    auto Var::CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return CreateBound();
    }

    auto Var::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Var::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Var::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::LocalVar;
    }

    auto Var::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto Var::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));
        return std::unique_ptr<ISymbol>
        {
            std::make_unique<LocalVarSymbol>(
                m_Scope,
                m_Name,
                typeSymbol
            )
        };
    }
}
