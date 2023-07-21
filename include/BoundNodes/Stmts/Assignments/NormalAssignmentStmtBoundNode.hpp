#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class NormalAssignmentStmtBoundNode :
        public std::enable_shared_from_this<NormalAssignmentStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<NormalAssignmentStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<NormalAssignmentStmtBoundNode>
    {
    public:
        NormalAssignmentStmtBoundNode(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<const IExprBoundNode>& t_lhsExpr,
            const std::shared_ptr<const IExprBoundNode>& t_rhsExpr
        );
        virtual ~NormalAssignmentStmtBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const NormalAssignmentStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const NormalAssignmentStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<const IExprBoundNode> m_LHSExpr{};
        std::shared_ptr<const IExprBoundNode> m_RHSExpr{};
    };
}
