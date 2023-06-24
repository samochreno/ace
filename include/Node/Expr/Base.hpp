#pragma once

#include <memory>

#include "Node/Base.hpp"
#include "BoundNode/Expr/Base.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expr
{
    class IBase : public virtual Node::IBase
    {
    public:
        virtual ~IBase() = default;
        
        virtual auto CloneInScopeExpr(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::IBase> = 0;

        virtual auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> = 0;
    };
}
