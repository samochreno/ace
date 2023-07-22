#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class ExprExprBoundNode :
        public std::enable_shared_from_this<ExprExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<ExprExprBoundNode>,
        public virtual ILowerableBoundNode<ExprExprBoundNode>
    {
    public:
        ExprExprBoundNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprBoundNode>& expr
        );
        virtual ~ExprExprBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ExprExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const ExprExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprBoundNode> m_Expr{};
    };
}
