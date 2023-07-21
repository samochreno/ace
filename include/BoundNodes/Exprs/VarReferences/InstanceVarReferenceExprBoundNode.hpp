#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class InstanceVarReferenceExprBoundNode :
        public std::enable_shared_from_this<InstanceVarReferenceExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<InstanceVarReferenceExprBoundNode>,
        public virtual ILowerableBoundNode<InstanceVarReferenceExprBoundNode>
    {
    public:
        InstanceVarReferenceExprBoundNode(
            const SourceLocation& sourceLocation,
            const std::shared_ptr<const IExprBoundNode>& expr,
            InstanceVarSymbol* const varSymbol
        );
        virtual ~InstanceVarReferenceExprBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const InstanceVarReferenceExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const InstanceVarReferenceExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetExpr() const -> std::shared_ptr<const IExprBoundNode>;
        auto GetVarSymbol() const -> InstanceVarSymbol*;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<const IExprBoundNode> m_Expr{};
        InstanceVarSymbol* m_VarSymbol{};
    };
}
