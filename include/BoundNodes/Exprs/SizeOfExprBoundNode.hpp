#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "TypeInfo.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class SizeOfExprBoundNode :
        public std::enable_shared_from_this<SizeOfExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<SizeOfExprBoundNode>,
        public virtual ILowerableBoundNode<SizeOfExprBoundNode>
    {
    public:
        SizeOfExprBoundNode(
            const std::shared_ptr<Scope>& t_scope,
            ITypeSymbol* const t_typeSymbol
        );
        virtual ~SizeOfExprBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const SizeOfExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const SizeOfExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        ITypeSymbol* m_TypeSymbol{};
    };
}
