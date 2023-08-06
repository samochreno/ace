#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "CFA.hpp"

namespace Ace
{
    class ReturnStmtBoundNode :
        public std::enable_shared_from_this<ReturnStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<ReturnStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<ReturnStmtBoundNode>
    {
    public:
        ReturnStmtBoundNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::optional<std::shared_ptr<const IExprBoundNode>>& optExpr
        );
        virtual ~ReturnStmtBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const ReturnStmtBoundNode>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const ReturnStmtBoundNode> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtBoundNode> final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateCFANodes() const -> std::vector<CFANode> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::optional<std::shared_ptr<const IExprBoundNode>> m_OptExpr{};
    };
}
