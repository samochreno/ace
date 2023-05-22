#pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression/SizeOf.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Error.hpp"

namespace Ace::Node::Expression
{
    class SizeOf :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::SizeOf>,
        public virtual Node::IBindable<BoundNode::Expression::SizeOf>
    {
    public:
        SizeOf(
            const std::shared_ptr<Scope>& t_scope,
            const TypeName& t_typeName
        ) : m_Scope{ t_scope}, 
            m_TypeName{ t_typeName }
        {
        }
        virtual ~SizeOf() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::SizeOf> final;
        auto CloneInScopeExpression(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::SizeOf>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<Scope> m_Scope{};
        TypeName m_TypeName{};
    };
}
