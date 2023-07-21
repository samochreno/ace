#include "BoundNodes/Stmts/ExpandableStmtBoundNode.hpp"

#include <memory>
#include <vector>

namespace Ace
{
    auto IExpandableStmtBoundNode::CreateExpanded() const -> std::vector<std::shared_ptr<const IStmtBoundNode>>
    {
        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        const auto partiallyExpanded = CreatePartiallyExpanded();
        std::for_each(begin(partiallyExpanded), end(partiallyExpanded),
        [&](const std::shared_ptr<const IStmtBoundNode>& stmt)
        {
            if (const auto* const expandable = dynamic_cast<const IExpandableStmtBoundNode*>(stmt.get()))
            {
                const auto expanded = expandable->CreateExpanded();
                stmts.insert(end(stmts), begin(expanded), end(expanded));
            }
            else
            {
                stmts.push_back(stmt);
            }
        });

        return stmts;
    }
}
