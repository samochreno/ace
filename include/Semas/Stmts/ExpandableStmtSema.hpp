#pragma once

#include <memory>
#include <vector>

#include "Semas/Stmts/StmtSema.hpp"

namespace Ace
{
    class IExpandableStmtSema : public virtual IStmtSema
    {
    public:
        virtual ~IExpandableStmtSema() = default;

        virtual auto CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtSema>> = 0;
        virtual auto CreateExpanded() const -> std::vector<std::shared_ptr<const IStmtSema>> final;
    };
}
