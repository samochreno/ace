#pragma once

#include <memory>

#include "Nodes/Node.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class IExprNode : public virtual INode
    {
    public:
        virtual ~IExprNode() = default;
        
        virtual auto CloneInScopeExpr(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IExprNode> = 0;

        virtual auto CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>> = 0;
    };
}
