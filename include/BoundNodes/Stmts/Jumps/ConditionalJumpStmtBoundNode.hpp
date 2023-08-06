#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "CFA.hpp"

namespace Ace
{
    class ConditionalJumpStmtBoundNode :
        public std::enable_shared_from_this<ConditionalJumpStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<ConditionalJumpStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<ConditionalJumpStmtBoundNode>
    {
    public:
        ConditionalJumpStmtBoundNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprBoundNode>& condition,
            LabelSymbol* const labelSymbol
        );
        virtual ~ConditionalJumpStmtBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const ConditionalJumpStmtBoundNode>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const ConditionalJumpStmtBoundNode> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtBoundNode> final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateCFANodes() const -> std::vector<CFANode> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprBoundNode> m_Condition{};
        LabelSymbol* m_LabelSymbol{};
    };
}
