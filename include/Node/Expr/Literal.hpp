#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Expr/Base.hpp"
#include "BoundNode/Expr/Literal.hpp"
#include "Scope.hpp"
#include "LiteralKind.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expr
{
    class Literal :
        public virtual Node::Expr::IBase,
        public virtual Node::ICloneable<Node::Expr::Literal>, 
        public virtual Node::IBindable<BoundNode::Expr::Literal>
    {
    public:
        Literal(
            const std::shared_ptr<Scope>& t_scope,
            const LiteralKind& t_kind,
            const std::string& t_string
        ) : m_Scope{ t_scope },
            m_Kind{ t_kind },
            m_String{ t_string }
        {
        }
        virtual ~Literal() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::Literal> final;
        auto CloneInScopeExpr(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::Literal>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<Scope> m_Scope{};
        LiteralKind m_Kind{};
        std::string m_String{};
    };
}
