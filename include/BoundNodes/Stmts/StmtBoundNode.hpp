#pragma once

#include <memory>

#include "BoundNodes/BoundNode.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Emittable.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    struct StmtTypeCheckingContext : public TypeCheckingContext
    {
        StmtTypeCheckingContext(
            ITypeSymbol* const t_parentFunctionType
        ) : ParentFunctionTypeSymbol{ t_parentFunctionType }
        {
        }

        ITypeSymbol* ParentFunctionTypeSymbol{};
    };

    class IStmtBoundNode :
        public virtual IBoundNode,
        public virtual IEmittable<void>
    {
    public:
        virtual ~IStmtBoundNode() = default;

        virtual auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> = 0;
        virtual auto GetOrCreateLoweredStmt(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> = 0;
    };
}
