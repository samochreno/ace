#pragma once

#include <memory>
#include <vector>

#include "Nodes/Node.hpp"
#include "BoundNode/Stmt/Base.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class IStmtNode : public virtual INode
    {
    public:
        virtual ~IStmtNode() = default;

        virtual auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IStmtNode> = 0;
        
        virtual auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> = 0;
    };
}
