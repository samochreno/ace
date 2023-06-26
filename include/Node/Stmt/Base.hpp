#pragma once

#include <memory>
#include <vector>

#include "Node/Base.hpp"
#include "BoundNode/Stmt/Base.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Stmt
{
    class IBase : public virtual Node::IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const Node::Stmt::IBase> = 0;
        
        virtual auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> = 0;
    };
}
