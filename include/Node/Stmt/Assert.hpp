#pragma once

#include <memory>
#include <vector>

#include "Node/Stmt/Base.hpp"
#include "Node/Expr/Base.hpp"
#include "BoundNode/Stmt/Assert.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Stmt
{
    class Assert :
        public virtual Node::Stmt::IBase,
        public virtual Node::ICloneable<Node::Stmt::Assert>,
        public virtual Node::IBindable<BoundNode::Stmt::Assert>
    {
    public:
        Assert(
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<const Node::Expr::IBase>& t_condition
        );
        virtual ~Assert() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::Assert> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::IBase> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Assert>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const Node::Expr::IBase> m_Condition{};
    };
}
