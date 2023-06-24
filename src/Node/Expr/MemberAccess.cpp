#include "Node/Expr/MemberAccess.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expr/VarReference/Instance.hpp"
#include "Diagnostics.hpp"
#include "Symbol/Var/Normal/Instance.hpp"
#include "Symbol/Type/Base.hpp"

namespace Ace::Node::Expr
{
    auto MemberAccess::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto MemberAccess::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::MemberAccess>
    {
        return std::make_shared<const Node::Expr::MemberAccess>(
            m_Expr->CloneInScopeExpr(t_scope),
            m_Name
        );
    }

    auto MemberAccess::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::VarReference::Instance>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());

        ACE_TRY_ASSERT(m_Name.TemplateArgs.empty());
        ACE_TRY(memberSymbol, GetScope()->ResolveInstanceSymbol<Symbol::Var::Normal::Instance>(
            boundExpr->GetTypeInfo().Symbol->GetWithoutReference(),
            m_Name
        ));

        return std::make_shared<const BoundNode::Expr::VarReference::Instance>(
            boundExpr,
            memberSymbol
        );
    }
}
