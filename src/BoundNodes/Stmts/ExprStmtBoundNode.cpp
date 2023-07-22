#include "BoundNodes/Stmts/ExprStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

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

    auto ExprStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto ExprStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ExprStmtBoundNode>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));
        
        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ExprStmtBoundNode>(
            GetSrcLocation(),
            mchCheckedExpr.Value
        ));
    }

    auto ExprStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto ExprStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const ExprStmtBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ExprStmtBoundNode>(
            GetSrcLocation(),
            mchLoweredExpr.Value
        )->GetOrCreateLowered(context).Value);
    }

    auto ExprStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto ExprStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        const auto exprEmitResult = m_Expr->Emit(emitter);
        
        emitter.EmitDropTmps(exprEmitResult.Tmps);
    }
}
