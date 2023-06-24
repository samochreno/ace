#pragma once

#include <memory>
#include <vector>

#include "Node/Expr/Base.hpp"
#include "BoundNode/Expr/Base.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expr
{
    class FunctionCall :
        public virtual Node::Expr::IBase,
        public virtual Node::ICloneable<Node::Expr::FunctionCall>,
        public virtual Node::IBindable<BoundNode::Expr::IBase>
    {
    public:
        FunctionCall(
            const std::shared_ptr<const Node::Expr::IBase>& t_expr,
            const std::vector<std::shared_ptr<const Node::Expr::IBase>>& t_args
        ) : m_Expr{ t_expr },
            m_Args{ t_args }
        {
        }
        virtual ~FunctionCall() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Expr->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::FunctionCall> final;
        auto CloneInScopeExpr(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<const Node::Expr::IBase> m_Expr{};
        std::vector<std::shared_ptr<const Node::Expr::IBase>> m_Args{};
    };
}
