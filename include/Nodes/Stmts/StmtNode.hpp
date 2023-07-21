#pragma once

#include <memory>
#include <vector>

#include "Nodes/Node.hpp"
#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class IStmtNode : public virtual INode
    {
    public:
        virtual ~IStmtNode() = default;

        virtual auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IStmtNode> = 0;
        
        virtual auto CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>> = 0;
    };
}
