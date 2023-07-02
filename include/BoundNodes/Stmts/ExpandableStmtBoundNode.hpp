#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"

namespace Ace
{
    class IExpandableStmtBoundNode : public virtual IStmtBoundNode
    {
    public:
        virtual ~IExpandableStmtBoundNode() = default;

        virtual auto CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtBoundNode>> = 0;
        virtual auto CreateExpanded() const -> std::vector<std::shared_ptr<const IStmtBoundNode>> final;
    };
}
