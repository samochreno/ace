#pragma once

#include <memory>
#include <vector>

#include "Node/Stmt/Base.hpp"
#include "Node/Expr/Base.hpp"
#include "Node/Stmt/Block.hpp"
#include "BoundNode/Stmt/While.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Stmt
{
    class While :
        public virtual Node::Stmt::IBase,
        public virtual Node::ICloneable<Node::Stmt::While>,
        public virtual Node::IBindable<BoundNode::Stmt::While>
    {
    public:
        While(
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<const Node::Expr::IBase>& t_condition,
            const std::shared_ptr<const Node::Stmt::Block>& t_body
        );
        virtual ~While() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::While> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::IBase> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::While>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const Node::Expr::IBase> m_Condition{};
        std::shared_ptr<const Node::Stmt::Block> m_Body{};
    };
}
