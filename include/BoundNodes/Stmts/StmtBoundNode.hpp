#pragma once

#include <memory>

#include "BoundNodes/BoundNode.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Emittable.hpp"
#include "Diagnostic.hpp"
#include "Cacheable.hpp"

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

        virtual auto CloneWithDiagnosticsStmt(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const IStmtBoundNode> = 0;
        virtual auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const IStmtBoundNode>>> = 0;
        virtual auto GetOrCreateLoweredStmt(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const IStmtBoundNode>> = 0;
    };
}
