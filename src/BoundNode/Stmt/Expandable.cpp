#include "BoundNode/Stmt/Expandable.hpp"

#include <memory>
#include <vector>

namespace Ace::BoundNode::Stmt
{
    auto IExpandable::CreateExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>> stmts{};

        const auto partiallyExpanded = CreatePartiallyExpanded();
        std::for_each(begin(partiallyExpanded), end(partiallyExpanded),
        [&](const std::shared_ptr<const BoundNode::Stmt::IBase>& t_stmt)
        {
            if (const auto* const expandable = dynamic_cast<const BoundNode::Stmt::IExpandable*>(t_stmt.get()))
            {
                const auto expanded = expandable->CreateExpanded();
                stmts.insert(end(stmts), begin(expanded), end(expanded));
            }
            else
            {
                stmts.push_back(t_stmt);
            }
        });

        return stmts;
    }
}
