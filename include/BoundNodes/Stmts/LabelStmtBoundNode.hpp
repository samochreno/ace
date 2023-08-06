#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "CFA.hpp"

namespace Ace
{
    class LabelStmtBoundNode :
        public std::enable_shared_from_this<LabelStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<LabelStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<LabelStmtBoundNode>
    {
    public:
        LabelStmtBoundNode(
            const SrcLocation& srcLocation,
            LabelSymbol* const symbol
        );
        virtual ~LabelStmtBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const LabelStmtBoundNode>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const LabelStmtBoundNode> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtBoundNode> final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateCFANodes() const -> std::vector<CFANode> final;

        auto GetSymbol() const -> LabelSymbol*;

    private:
        SrcLocation m_SrcLocation{};
        LabelSymbol* m_Symbol{};
    };
}
