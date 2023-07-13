#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/TypedBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"

namespace Ace
{
    class VarStmtBoundNode :
        public std::enable_shared_from_this<VarStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypedBoundNode<LocalVarSymbol>,
        public virtual ITypeCheckableBoundNode<VarStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<VarStmtBoundNode>
    {
    public:
        VarStmtBoundNode(
            LocalVarSymbol* const t_symbol,
            const std::optional<std::shared_ptr<const IExprBoundNode>>& t_optAssignedExpr
        );
        virtual ~VarStmtBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const VarStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const VarStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

        auto GetSymbol() const -> LocalVarSymbol* final;
        
    private:
        LocalVarSymbol* m_Symbol{};
        std::optional<std::shared_ptr<const IExprBoundNode>> m_OptAssignedExpr{};
    };
}
