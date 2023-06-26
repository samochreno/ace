#pragma once

#include <memory>
#include <vector>

#include "Node/Stmt/Base.hpp"
#include "Node/Expr/Base.hpp"
#include "BoundNode/Stmt/Assignment/Compound.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Stmt::Assignment
{
    class Compound :
        public virtual Node::Stmt::IBase,
        public virtual Node::ICloneable<Node::Stmt::Assignment::Compound>,
        public virtual Node::IBindable<BoundNode::Stmt::Assignment::Compound>
    {
    public:
        Compound(
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<const Node::Expr::IBase>& t_lhsExpr,
            const std::shared_ptr<const Node::Expr::IBase>& t_rhsExpr,
            const TokenKind& t_operator
        );
        virtual ~Compound() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::Assignment::Compound> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::IBase> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Assignment::Compound>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const Node::Expr::IBase> m_LHSExpr{};
        std::shared_ptr<const Node::Expr::IBase> m_RHSExpr{};
        TokenKind m_Operator{};
    };
}
