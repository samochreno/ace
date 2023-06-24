#pragma once

#include <memory>
#include <vector>

#include "Node/Expr/Base.hpp"
#include "BoundNode/Expr//Or.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expr
{
    class Or :
        public virtual Node::Expr::IBase,
        public virtual Node::ICloneable<Node::Expr::Or>,
        public virtual Node::IBindable<BoundNode::Expr::Or>
    {
    public:
        Or(
            const std::shared_ptr<const Node::Expr::IBase>& t_lhsExpr,
            const std::shared_ptr<const Node::Expr::IBase>& t_rhsExpr
        ) : m_LHSExpr{ t_lhsExpr },
            m_RHSExpr{ t_rhsExpr }
        {
        }
        virtual ~Or() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_LHSExpr->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::Or> final;
        auto CloneInScopeExpr(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::Or>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<const Node::Expr::IBase> m_LHSExpr{};
        std::shared_ptr<const Node::Expr::IBase> m_RHSExpr{};
    };
}
