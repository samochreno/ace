#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class ExprStmtBoundNode : 
        public std::enable_shared_from_this<ExprStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<ExprStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<ExprStmtBoundNode>
    {
    public:
        ExprStmtBoundNode(
            const std::shared_ptr<const IExprBoundNode>& t_expr
        );
        virtual ~ExprStmtBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ExprStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const ExprStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

    private:
        std::shared_ptr<const IExprBoundNode> m_Expr{};
    };
}
