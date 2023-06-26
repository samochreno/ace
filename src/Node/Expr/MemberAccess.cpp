#include "Node/Expr/MemberAccess.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expr/VarReference/Instance.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace::Node::Expr
{
    MemberAccess::MemberAccess(
        const std::shared_ptr<const Node::Expr::IBase>& t_expr,
        const SymbolNameSection& t_name
    ) : m_Expr{ t_expr },
        m_Name{ t_name }
    {
    }

    auto MemberAccess::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto MemberAccess::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto MemberAccess::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::MemberAccess>
    {
        return std::make_shared<const Node::Expr::MemberAccess>(
            m_Expr->CloneInScopeExpr(t_scope),
            m_Name
        );
    }

    auto MemberAccess::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto MemberAccess::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::VarReference::Instance>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());

        ACE_TRY_ASSERT(m_Name.TemplateArgs.empty());
        ACE_TRY(memberSymbol, GetScope()->ResolveInstanceSymbol<InstanceVarSymbol>(
            boundExpr->GetTypeInfo().Symbol->GetWithoutReference(),
            m_Name
        ));

        return std::make_shared<const BoundNode::Expr::VarReference::Instance>(
            boundExpr,
            memberSymbol
        );
    }

    auto MemberAccess::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }

    auto MemberAccess::GetExpr() const -> const Node::Expr::IBase*
    {
        return m_Expr.get();
    }

    auto MemberAccess::GetName() const -> const SymbolNameSection&
    {
        return m_Name;
    }
}
