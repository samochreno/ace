#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class InstanceVarRefExprBoundNode :
        public std::enable_shared_from_this<InstanceVarRefExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<InstanceVarRefExprBoundNode>,
        public virtual ILowerableBoundNode<InstanceVarRefExprBoundNode>
    {
    public:
        InstanceVarRefExprBoundNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprBoundNode>& expr,
            InstanceVarSymbol* const varSymbol
        );
        virtual ~InstanceVarRefExprBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const InstanceVarRefExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const InstanceVarRefExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetExpr() const -> std::shared_ptr<const IExprBoundNode>;
        auto GetVarSymbol() const -> InstanceVarSymbol*;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprBoundNode> m_Expr{};
        InstanceVarSymbol* m_VarSymbol{};
    };
}
