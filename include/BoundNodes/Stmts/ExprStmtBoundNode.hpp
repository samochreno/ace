#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    class ExprStmtBoundNode : 
        public std::enable_shared_from_this<ExprStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ICloneableWithDiagnosticsBoundNode<ExprStmtBoundNode>,
        public virtual ITypeCheckableBoundNode<ExprStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<ExprStmtBoundNode>
    {
    public:
        ExprStmtBoundNode(
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprBoundNode>& expr
        );
        virtual ~ExprStmtBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CloneWithDiagnostics(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const ExprStmtBoundNode> final;
        auto CloneWithDiagnosticsStmt(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const IStmtBoundNode> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const ExprStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const ExprStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& emitter) const -> void final;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprBoundNode> m_Expr{};
    };
}
