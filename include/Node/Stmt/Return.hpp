#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Node/Stmt/Base.hpp"
#include "Node/Expr/Base.hpp"
#include "BoundNode/Stmt/Return.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Stmt
{
    class Return :
        public virtual Node::Stmt::IBase,
        public virtual Node::ICloneable<Node::Stmt::Return>,
        public virtual Node::IBindable<BoundNode::Stmt::Return>
    {
    public:
        Return(
            const std::shared_ptr<Scope>& t_scope,
            const std::optional<std::shared_ptr<const Node::Expr::IBase>>& t_optExpr
        );
        virtual ~Return() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::Return> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::IBase> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Return>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::optional<std::shared_ptr<const Node::Expr::IBase>> m_OptExpr{};
    };
}
