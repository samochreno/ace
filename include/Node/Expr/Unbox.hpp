#pragma once

#include <memory>
#include <vector>

#include "Node/Expr/Base.hpp"
#include "BoundNode/Expr/Unbox.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expr
{
    class Unbox :
        public virtual Node::Expr::IBase,
        public virtual Node::ICloneable<Node::Expr::Unbox>,
        public virtual Node::IBindable<BoundNode::Expr::Unbox>
    {
    public:
        Unbox(const std::shared_ptr<const Node::Expr::IBase>& t_expr)
            : m_Expr{ t_expr }
        {
        }
        virtual ~Unbox() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Expr->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::Unbox> final;
        auto CloneInScopeExpr(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::Unbox>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<const Node::Expr::IBase> m_Expr{};
    };
}
