#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "Assert.hpp"
#include "Diagnostic.hpp"
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
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const TypeInfo& typeInfo
        );
        virtual ~ConversionPlaceholderExprBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ConversionPlaceholderExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const ConversionPlaceholderExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        TypeInfo m_TypeInfo;
    };
}
