#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

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
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ReturnStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const ReturnStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& emitter) const -> void final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::optional<std::shared_ptr<const IExprBoundNode>> m_OptExpr{};
    };
}
