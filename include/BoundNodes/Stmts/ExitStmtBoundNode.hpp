#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class ExitStmtBoundNode : 
        public std::enable_shared_from_this<ExitStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<ExitStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<ExitStmtBoundNode>
    {
    public:
        ExitStmtBoundNode(const std::shared_ptr<Scope>& t_scope);
        virtual ~ExitStmtBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ExitStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const ExitStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

    private:
        std::shared_ptr<Scope> m_Scope{};
    };
}
