#pragma once

#include <memory>
#include <vector>

#include "Semas/Sema.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Emittable.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    struct StmtTypeCheckingContext
    {
        ITypeSymbol* ParentFunctionTypeSymbol{};
    };

    class IStmtSema :
        public virtual ISema,
        public virtual IEmittable<void>
    {
    public:
        virtual ~IStmtSema() = default;

        virtual auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtSema>> = 0;
        virtual auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtSema> = 0;

        virtual auto CreateControlFlowNodes() const -> std::vector<ControlFlowNode> = 0;
    };
}
