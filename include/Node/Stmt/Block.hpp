#pragma once

#include <memory>
#include <vector>

#include "Node/Stmt/Base.hpp"
#include "BoundNode/Stmt/Block.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Stmt
{
    class Block :
        public virtual Node::Stmt::IBase,
        public virtual Node::ICloneable<Node::Stmt::Block>,
        public virtual Node::IBindable<BoundNode::Stmt::Block>
    {
    public:
        Block(
            const std::shared_ptr<Scope>& t_selfScope,
            const std::vector<std::shared_ptr<const Node::Stmt::IBase>>& t_stmts
        );
        virtual ~Block() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::Block> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::IBase> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Block>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        std::vector<std::shared_ptr<const Node::Stmt::IBase>> m_Stmts{};
    };
}
