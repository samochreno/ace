#pragma once

#include <memory>
#include <vector>

#include "Node/Stmt/Base.hpp"
#include "Node/Expr/Base.hpp"
#include "BoundNode/Stmt/Assignment/Normal.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Stmt::Assignment
{
    class Normal :
        public virtual Node::Stmt::IBase,
        public virtual Node::ICloneable<Node::Stmt::Assignment::Normal>,
        public virtual Node::IBindable<BoundNode::Stmt::Assignment::Normal>
    {
    public:
        Normal(
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<const Node::Expr::IBase>& t_lhsExpr,
            const std::shared_ptr<const Node::Expr::IBase>& t_rhsExpr
        ) : m_Scope{ t_scope },
            m_LHSExpr{ t_lhsExpr },
            m_RHSExpr{ t_rhsExpr }
        {
        }
        virtual ~Normal() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::Assignment::Normal> final;
        auto CloneInScopeStmt(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Assignment::Normal>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final { return CreateBound(); }
        
    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const Node::Expr::IBase> m_LHSExpr{};
        std::shared_ptr<const Node::Expr::IBase> m_RHSExpr{};
    };
}
