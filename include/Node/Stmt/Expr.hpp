#pragma once

#include <memory>
#include <vector>

#include "Node/Stmt/Base.hpp"
#include "Node/Expr/Base.hpp"
#include "BoundNode/Stmt/Expr.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Stmt
{
    class Expr :
        public virtual Node::Stmt::IBase,
        public virtual Node::ICloneable<Node::Stmt::Expr>,
        public virtual Node::IBindable<BoundNode::Stmt::Expr>
    {
    public:
        Expr(const std::shared_ptr<const Node::Expr::IBase>& t_expr)
            : m_Expr{ t_expr }
        {
        }
        virtual ~Expr() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Expr->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::Expr> final;
        auto CloneInScopeStmt(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Expr>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<const Node::Expr::IBase> m_Expr{};
    };
}
