#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Stmt/Base.hpp"

namespace Ace::BoundNode::Stmt
{
    class IExpandable : public virtual BoundNode::Stmt::IBase
    {
    public:
        virtual ~IExpandable() = default;

        virtual auto CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>> = 0;
        virtual auto CreateExpanded() const -> std::vector<std::shared_ptr<const BoundNode::Stmt::IBase>> final;
    };
}
