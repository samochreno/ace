#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression/Literal.hpp"
#include "Scope.hpp"
#include "LiteralKind.hpp"
#include "Error.hpp"

namespace Ace::Node::Expression
{
    class Literal :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::Literal>, 
        public virtual Node::IBindable<BoundNode::Expression::Literal>
    {
    public:
        Literal(
            Scope* const t_scope,
            const LiteralKind& t_kind,
            const std::string& t_string
        ) : m_Scope{ t_scope },
            m_Kind{ t_kind },
            m_String{ t_string }
        {
        }
        virtual ~Literal() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::Literal> final;
        auto CloneInScopeExpression(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::Literal>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }

    private:
        Scope* m_Scope{};
        LiteralKind m_Kind{};
        std::string m_String{};
    };
}
