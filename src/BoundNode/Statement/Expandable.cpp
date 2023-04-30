#include "BoundNode/Statement/Expandable.hpp"

#include <memory>
#include <vector>

namespace Ace::BoundNode::Statement
{
    auto IExpandable::CreateExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Statement::IBase>>
    {
        std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> statements{};

        const auto partiallyExpanded = CreatePartiallyExpanded();
        std::for_each(begin(partiallyExpanded), end(partiallyExpanded),
        [&](const std::shared_ptr<const BoundNode::Statement::IBase>& t_statement)
        {
            if (const auto* const expandable = dynamic_cast<const BoundNode::Statement::IExpandable*>(t_statement.get()))
            {
                const auto expanded = expandable->CreateExpanded();
                statements.insert(end(statements), begin(expanded), end(expanded));
            }
            else
            {
                statements.push_back(t_statement);
            }
        });

        return statements;
    }
}
