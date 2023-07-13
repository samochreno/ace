#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class AndExprBoundNode :
        public std::enable_shared_from_this<AndExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<AndExprBoundNode>,
        public virtual ILowerableBoundNode<AndExprBoundNode>
    {
    public:
        AndExprBoundNode(
            const std::shared_ptr<const IExprBoundNode>& t_lhsExpr,
            const std::shared_ptr<const IExprBoundNode>& t_rhsExpr
        );
        virtual ~AndExprBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const AndExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const AndExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;

        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;
    
    private:
        std::shared_ptr<const IExprBoundNode> m_LHSExpr;
        std::shared_ptr<const IExprBoundNode> m_RHSExpr;
    };
}
