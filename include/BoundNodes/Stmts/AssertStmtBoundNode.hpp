#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Stmts/GroupStmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"
#include "Assert.hpp"

namespace Ace
{
    class AssertStmtBoundNode : 
        public std::enable_shared_from_this<AssertStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ICloneableWithDiagnosticsBoundNode<AssertStmtBoundNode>,
        public virtual ITypeCheckableBoundNode<AssertStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<GroupStmtBoundNode>
    {
    public:
        AssertStmtBoundNode(
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprBoundNode>& condition
        );
        virtual ~AssertStmtBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CloneWithDiagnostics(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const AssertStmtBoundNode> final;
        auto CloneWithDiagnosticsStmt(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const IStmtBoundNode> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const AssertStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const GroupStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& emitter) const -> void final;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprBoundNode> m_Condition{};
    };
}
