#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class AssignmentStmtBoundNode :
        public std::enable_shared_from_this<AssignmentStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<AssignmentStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<AssignmentStmtBoundNode>
    {
    public:
        AssignmentStmtBoundNode(
            const std::shared_ptr<const IExprBoundNode>& t_lhsExpr,
            const std::shared_ptr<const IExprBoundNode>& t_rhsExpr
        );
        virtual ~AssignmentStmtBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const AssignmentStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const AssignmentStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

    private:
        std::shared_ptr<const IExprBoundNode> m_LHSExpr{};
        std::shared_ptr<const IExprBoundNode> m_RHSExpr{};
    };
}
