#pragma once

#include <memory>

#include "Node/Base.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expression
{
    class IBase : public virtual Node::IBase
    {
    public:
        virtual ~IBase() = default;
        
        virtual auto CloneInScopeExpression(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::IBase> = 0;

        virtual auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> = 0;
    };
}
