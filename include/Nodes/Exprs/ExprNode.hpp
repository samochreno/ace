#pragma once

#include <memory>

#include "Nodes/Node.hpp"
#include "BoundNode/Expr/Base.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class IExprNode : public virtual INode
    {
    public:
        virtual ~IExprNode() = default;
        
        virtual auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> = 0;

        virtual auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> = 0;
    };
}
