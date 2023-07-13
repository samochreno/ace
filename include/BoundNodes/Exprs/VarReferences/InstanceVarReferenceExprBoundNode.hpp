#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
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
            const std::shared_ptr<const IExprBoundNode>& t_expr,
            InstanceVarSymbol* const t_variableSymbol
        );
        virtual ~InstanceVarReferenceExprBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const InstanceVarReferenceExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const InstanceVarReferenceExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetExpr() const -> std::shared_ptr<const IExprBoundNode>;
        auto GetVarSymbol() const -> InstanceVarSymbol*;

    private:
        std::shared_ptr<const IExprBoundNode> m_Expr{};
        InstanceVarSymbol* m_VarSymbol{};
    };
}
