#include "Semas/Stmts/ExpandableStmtSema.hpp"

#include <memory>
#include <vector>

namespace Ace
{
    auto IExpandableStmtSema::CreateExpanded() const -> std::vector<std::shared_ptr<const IStmtSema>>
    {
        std::vector<std::shared_ptr<const IStmtSema>> stmts{};

        const auto partiallyExpanded = CreatePartiallyExpanded();
        std::for_each(begin(partiallyExpanded), end(partiallyExpanded),
        [&](const std::shared_ptr<const IStmtSema>& stmt)
        {
            if (const auto* const expandable = dynamic_cast<const IExpandableStmtSema*>(stmt.get()))
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
