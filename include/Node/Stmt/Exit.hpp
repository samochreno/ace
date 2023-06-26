#pragma once

#include <memory>
#include <vector>

#include "Node/Stmt/Base.hpp"
#include "BoundNode/Stmt/Exit.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Stmt
{
    class Exit :
        public virtual Node::Stmt::IBase,
        public virtual Node::ICloneable<Node::Stmt::Exit>,
        public virtual Node::IBindable<BoundNode::Stmt::Exit>
    {
    public:
        Exit(const std::shared_ptr<Scope>& t_scope);
        virtual ~Exit() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::Exit> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::IBase> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Exit>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
    };
}
