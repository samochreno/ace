#include "BoundNodes/Stmts/ExprStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "CFA.hpp"

namespace Ace
{
    ExprStmtBoundNode::ExprStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_Expr{ expr }
    {
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

    auto ExprStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ExprStmtBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto dgnCheckedExpr = m_Expr->CreateTypeCheckedExpr({});
        diagnostics.Add(dgnCheckedExpr);

        if (dgnCheckedExpr.Unwrap() == m_Expr)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const ExprStmtBoundNode>(
                GetSrcLocation(),
                dgnCheckedExpr.Unwrap()
            ),
            diagnostics,
        };
    }

    auto ExprStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto ExprStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const ExprStmtBoundNode>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const ExprStmtBoundNode>(
            GetSrcLocation(),
            loweredExpr
        )->CreateLowered({});
    }

    auto ExprStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto ExprStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        const auto exprEmitResult = m_Expr->Emit(emitter);
        
        emitter.EmitDropTmps(exprEmitResult.Tmps);
    }

    auto ExprStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        return {};
    }
}
