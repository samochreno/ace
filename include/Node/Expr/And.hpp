 #pragma once

#include <memory>
#include <vector>

#include "Node/Expr/Base.hpp"
#include "BoundNode/Expr//And.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expr
{
    class And :
        public virtual Node::Expr::IBase,
        public virtual Node::ICloneable<Node::Expr::And>,
        public virtual Node::IBindable<BoundNode::Expr::And>
    {
    public:
        And(
            const std::shared_ptr<const Node::Expr::IBase>& t_lhsExpr,
            const std::shared_ptr<const Node::Expr::IBase>& t_rhsExpr
        );
        virtual ~And() = default;
    
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Expr::And> final;
        auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Expr::IBase> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::And>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final;

    private:
        std::shared_ptr<const Node::Expr::IBase> m_LHSExpr{};
        std::shared_ptr<const Node::Expr::IBase> m_RHSExpr{};
    };
}
