#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "MaybeChanged.hpp"
#include "Assert.hpp"

namespace Ace
{
    class IfStmtBoundNode :
        public std::enable_shared_from_this<IfStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<IfStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<GroupStmtBoundNode>
    {
    public:
        IfStmtBoundNode(
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::vector<std::shared_ptr<const IExprBoundNode>>& conditions,
            const std::vector<std::shared_ptr<const BlockStmtBoundNode>>& bodies
        );
        virtual ~IfStmtBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IfStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const GroupStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& emitter) const -> void final;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::vector<std::shared_ptr<const IExprBoundNode>> m_Conditions{};
        std::vector<std::shared_ptr<const BlockStmtBoundNode>> m_Bodies{};
    };
}
