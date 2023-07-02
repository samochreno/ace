#pragma once

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"

namespace Ace
{
    class IJumpStmtBoundNode : public virtual IStmtBoundNode
    {
    public:
        virtual ~IJumpStmtBoundNode() = default;

        virtual auto GetLabelSymbol() const -> LabelSymbol* = 0;
    };
}
