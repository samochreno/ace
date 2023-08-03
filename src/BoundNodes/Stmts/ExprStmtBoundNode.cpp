#include "BoundNodes/Stmts/ExprStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Cacheable.hpp"
#include "Emitter.hpp"

namespace Ace
{
    ExprStmtBoundNode::ExprStmtBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_Diagnostics{ diagnostics },
        m_Expr{ expr }
    {
    }

    auto ExprStmtBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto ExprStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ExprStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto ExprStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto ExprStmtBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const ExprStmtBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const ExprStmtBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_Expr
        );
    }

    auto ExprStmtBoundNode::CloneWithDiagnosticsStmt(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto ExprStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const ExprStmtBoundNode>>>
    {
        ACE_TRY(cchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));
        
        if (!cchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ExprStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchCheckedExpr.Value
        ));
    }

    auto ExprStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto ExprStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const ExprStmtBoundNode>>
    {
        const auto cchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!cchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ExprStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchLoweredExpr.Value
        )->GetOrCreateLowered(context).Value);
    }

    auto ExprStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto ExprStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        const auto exprEmitResult = m_Expr->Emit(emitter);
        
        emitter.EmitDropTmps(exprEmitResult.Tmps);
    }
}
