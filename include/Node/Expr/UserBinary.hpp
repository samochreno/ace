#pragma once

#include <memory>
#include <vector>

#include "Node/Expr/Base.hpp"
#include "BoundNode/Expr/UserBinary.hpp"
#include "Scope.hpp"
#include "Token.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expr
{
    class UserBinary :
        public virtual Node::Expr::IBase,
        public virtual Node::ICloneable<Node::Expr::UserBinary>,
        public virtual Node::IBindable<BoundNode::Expr::UserBinary>
    {
    public:
        UserBinary(
            const std::shared_ptr<const Node::Expr::IBase>& t_lhsExpr,
            const std::shared_ptr<const Node::Expr::IBase>& t_rhsExpr,
            const TokenKind& t_operator
        ) : m_LHSExpr{ t_lhsExpr },
            m_RHSExpr{ t_rhsExpr },
            m_Operator{ t_operator }
        {
        }
        virtual ~UserBinary() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_LHSExpr->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::UserBinary> final;
        auto CloneInScopeExpr(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::UserBinary>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<const Node::Expr::IBase> m_LHSExpr{};
        std::shared_ptr<const Node::Expr::IBase> m_RHSExpr{};
        TokenKind m_Operator{};
    };
}
