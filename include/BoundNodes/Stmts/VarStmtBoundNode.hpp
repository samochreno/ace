#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/TypedBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
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
            const SrcLocation& srcLocation,
            LocalVarSymbol* const symbol,
            const std::optional<std::shared_ptr<const IExprBoundNode>>& optAssignedExpr
        );
        virtual ~VarStmtBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const VarStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const VarStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& emitter) const -> void final;

        auto GetSymbol() const -> LocalVarSymbol* final;
        
    private:
        SrcLocation m_SrcLocation{};
        LocalVarSymbol* m_Symbol{};
        std::optional<std::shared_ptr<const IExprBoundNode>> m_OptAssignedExpr{};
    };
}
