#pragma once

#include <memory>
#include <vector>

#include "Node/Expr/Base.hpp"
#include "BoundNode/Expr/VarReference/Instance.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expr
{
    class MemberAccess :
        public virtual Node::Expr::IBase,
        public virtual Node::ICloneable<Node::Expr::MemberAccess>,
        public virtual Node::IBindable<BoundNode::Expr::VarReference::Instance>
    {
    public:
        MemberAccess(
            const std::shared_ptr<const Node::Expr::IBase>& t_expr,
            const SymbolNameSection& t_name
        ) : m_Expr{ t_expr },
            m_Name{ t_name }
        {
        }
        virtual ~MemberAccess() = default;

        auto GetScope() const -> std::shared_ptr<Scope> { return m_Expr->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::MemberAccess> final;
        auto CloneInScopeExpr(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::VarReference::Instance>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final { return CreateBound(); }

        auto GetExpr() const -> const Node::Expr::IBase* { return m_Expr.get(); }
        auto GetName() const -> const SymbolNameSection& { return m_Name; }

    private:
        std::shared_ptr<const Node::Expr::IBase> m_Expr{};
        SymbolNameSection m_Name{};
    };
}
