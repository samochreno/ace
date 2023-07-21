#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class DereferenceExprBoundNode :
        public std::enable_shared_from_this<DereferenceExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<DereferenceExprBoundNode>,
        public virtual ILowerableBoundNode<DereferenceExprBoundNode>
    {
    public:
        DereferenceExprBoundNode(
            const SourceLocation& sourceLocation,
            const std::shared_ptr<const IExprBoundNode>& expr
        );
        virtual ~DereferenceExprBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const DereferenceExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const DereferenceExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetExpr() const -> std::shared_ptr<const IExprBoundNode>;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<const IExprBoundNode> m_Expr{};
    };
}
