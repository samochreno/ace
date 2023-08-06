#pragma once

#include <memory>

#include "BoundNodes/BoundNode.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Emittable.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    struct StmtTypeCheckingContext
    {
        ITypeSymbol* ParentFunctionTypeSymbol{};
    };

    class IStmtBoundNode :
        public virtual IBoundNode,
        public virtual IEmittable<void>
    {
    public:
        virtual ~IStmtBoundNode() = default;

        virtual auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>> = 0;
        virtual auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtBoundNode> = 0;
    };
}
