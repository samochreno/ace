#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "CFA.hpp"

namespace Ace
{
    class SimpleAssignmentStmtBoundNode :
        public std::enable_shared_from_this<SimpleAssignmentStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<SimpleAssignmentStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<SimpleAssignmentStmtBoundNode>
    {
    public:
        SimpleAssignmentStmtBoundNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprBoundNode>& lhsExpr,
            const std::shared_ptr<const IExprBoundNode>& rhsExpr
        );
        virtual ~SimpleAssignmentStmtBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const SimpleAssignmentStmtBoundNode>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const SimpleAssignmentStmtBoundNode> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtBoundNode> final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateCFANodes() const -> std::vector<CFANode> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprBoundNode> m_LHSExpr{};
        std::shared_ptr<const IExprBoundNode> m_RHSExpr{};
    };
}