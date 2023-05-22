#pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression/FunctionCall/Static.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Error.hpp"

namespace Ace::Node::Expression
{
    class Cast :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::Cast>,
        public virtual Node::IBindable<BoundNode::Expression::IBase>
    {
    public:
        Cast(
            const TypeName& t_typeName,
            const std::shared_ptr<const Node::Expression::IBase>& t_expression
        ) : m_TypeName{ t_typeName },
            m_Expression{ t_expression }
        {
        }
        virtual ~Cast() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Expression->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::Cast> final;
        auto CloneInScopeExpression(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }

    private:
        TypeName m_TypeName{};
        std::shared_ptr<const Node::Expression::IBase> m_Expression{};
    };
}
