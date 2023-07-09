#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "Assert.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class ConversionPlaceholderExprBoundNode :
        public std::enable_shared_from_this<ConversionPlaceholderExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<ConversionPlaceholderExprBoundNode>,
        public virtual ILowerableBoundNode<ConversionPlaceholderExprBoundNode>
    {
    public:
        ConversionPlaceholderExprBoundNode(
            const std::shared_ptr<Scope>& t_scope,
            const TypeInfo& t_typeInfo
        );
        virtual ~ConversionPlaceholderExprBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ConversionPlaceholderExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const ConversionPlaceholderExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        TypeInfo m_TypeInfo;
    };
}
