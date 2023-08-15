#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "CFA.hpp"

namespace Ace
{
    class CopyStmtBoundNode : 
        public std::enable_shared_from_this<CopyStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<CopyStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<CopyStmtBoundNode>
    {
    public:
        CopyStmtBoundNode(
            const SrcLocation& srcLocation,
            ISizedTypeSymbol* const typeSymbol,
            const std::shared_ptr<const IExprBoundNode>& srcExpr,
            const std::shared_ptr<const IExprBoundNode>& dstExpr
        );
        virtual ~CopyStmtBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const CopyStmtBoundNode>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const CopyStmtBoundNode> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtBoundNode> final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateCFANodes() const -> std::vector<CFANode> final;

    private:
        SrcLocation m_SrcLocation{};
        ISizedTypeSymbol* m_TypeSymbol{};
        std::shared_ptr<const IExprBoundNode> m_SrcExpr{};
        std::shared_ptr<const IExprBoundNode> m_DstExpr{};
    };
}
