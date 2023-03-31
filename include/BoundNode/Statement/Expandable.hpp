#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Statement/Base.hpp"

namespace Ace::BoundNode::Statement
{
    class IExpandable : public virtual BoundNode::Statement::IBase
    {
    public:
        virtual ~IExpandable() = default;

        virtual auto CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> = 0;
        virtual auto CreateExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> final;
    };
}
