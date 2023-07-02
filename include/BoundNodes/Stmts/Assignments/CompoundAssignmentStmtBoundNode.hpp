#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Asserts.hpp"

namespace Ace
{
    class CompoundAssignmentStmtBoundNode :
        public std::enable_shared_from_this<CompoundAssignmentStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<CompoundAssignmentStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<GroupStmtBoundNode>
    {
    public:
        CompoundAssignmentStmtBoundNode(
            const std::shared_ptr<const IExprBoundNode>& t_lhsExpr,
            const std::shared_ptr<const IExprBoundNode>& t_rhsExpr,
            FunctionSymbol* const t_operatorSymbol
        );
        virtual ~CompoundAssignmentStmtBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const CompoundAssignmentStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const GroupStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

    private:
        std::shared_ptr<const IExprBoundNode> m_LHSExpr{};
        std::shared_ptr<const IExprBoundNode> m_RHSExpr{};
        FunctionSymbol* m_OperatorSymbol{};
    };
}
