#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Node/Stmt/Base.hpp"
#include "Node/Expr/Base.hpp"
#include "Node/Stmt/Block.hpp"
#include "BoundNode/Stmt/If.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Stmt
{
    class If :
        public virtual Node::Stmt::IBase,
        public virtual Node::ICloneable<Node::Stmt::If>,
        public virtual Node::IBindable<BoundNode::Stmt::If>
    {
    public:
        If(
            const std::shared_ptr<Scope>& t_scope,
            const std::vector<std::shared_ptr<const Node::Expr::IBase>>& t_conditions,
            const std::vector<std::shared_ptr<const Node::Stmt::Block>>& t_bodies
        ) : m_Scope{ t_scope },
            m_Conditions{ t_conditions },
            m_Bodies{ t_bodies }

        {
        }
        virtual ~If() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::If> final;
        auto CloneInScopeStmt(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::If>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::vector<std::shared_ptr<const Node::Expr::IBase>> m_Conditions{};
        std::vector<std::shared_ptr<const Node::Stmt::Block>> m_Bodies{};
    };
}
